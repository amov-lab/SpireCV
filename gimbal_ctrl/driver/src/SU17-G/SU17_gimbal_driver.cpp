/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 10:12:58
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:58:10
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/SU17-G/SU17_gimbal_driver.cpp
 */

#include "SU17_gimbal_driver.h"
#include "SU17_gimbal_crc.h"
#include "string.h"

/**
 * The function creates a new instance of the su17GimbalDriver class, which is a subclass of the
 * IamovGimbalBase class
 *
 * @param _IO The IOStreamBase class that is used to communicate with the gimbal.
 */
su17GimbalDriver::su17GimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::amovGimbalBase(_IO)
{

    rxQueue = new fifoRing(sizeof(SU17::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);
    txQueue = new fifoRing(sizeof(SU17::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);

    parserState = SU17::GIMBAL_SERIAL_STATE_IDEL;

    union
    {
        uint16_t time;
        uint8_t data[2];
    } detalT;

    detalT.time = 50;

    pack(SU17::GIMBAL_CMD_ATT, detalT.data, 2);
    std::thread heart(&su17GimbalDriver::heart, this);
    this->heartThreadHandle = heart.native_handle();
    heart.detach();
}

void su17GimbalDriver::heart()
{
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        pack(SU17::GIMBAL_CMD_HEART, nullptr, 0);
    }
}

/**
 * It takes a command, a pointer to a payload, and the size of the payload, and then it puts the
 * command, the payload, and the CRC into a ring buffer
 *
 * @param uint32_t 4 bytes
 * @param pPayload pointer to the data to be sent
 * @param payloadSize the size of the payload in bytes
 *
 * @return The number of bytes in the packet.
 */
uint32_t su17GimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;
    SU17::GIMBAL_FRAME_T txTemp;

    txTemp.head = SU17_SERIAL_HEAD;
    txTemp.version = SU17_SERIAL_VERSION;
    txTemp.len = payloadSize;
    txTemp.command = cmd;
    txTemp.source = self;
    txTemp.target = remote;
    if (cmd == SU17::GIMBAL_CMD_TAKEPIC || cmd == SU17::GIMBAL_CMD__VIDEO)
    {
        txTemp.target++;
    }
    else if (cmd == SU17::GIMBAL_CMD_HEART)
    {
        txTemp.target = 0XFF;
    }

    memcpy(txTemp.data, pPayload, payloadSize);
    txTemp.crc.f16 = SU17::checkCrc16((uint8_t *)&txTemp, txTemp.len + SU17_PAYLOAD_OFFSET);
    memcpy(txTemp.data + payloadSize, txTemp.crc.f8, sizeof(uint16_t));

    if (txQueue->inCell(&txTemp))
    {
        ret = txTemp.len + SU17_PAYLOAD_OFFSET + sizeof(uint16_t);
    }

    return ret;
}

/**
 * The function takes a pointer to a buffer, casts it to a pointer to a SU17::GIMBAL_FRAME_T, and then
 * checks the command field of the frame. If the command is SU17::IAP_COMMAND_BLOCK_END, it locks the
 * mutex, and then unlocks it. Otherwise, it prints out the contents of the buffer
 *
 * @param buf pointer to the data received from the gimbal
 */
void su17GimbalDriver::convert(void *buf)
{
    SU17::GIMBAL_FRAME_T *temp;
    temp = reinterpret_cast<SU17::GIMBAL_FRAME_T *>(buf);

    switch (temp->command)
    {
    case SU17::GIMBAL_CMD_ATT:
        SU17::GIMBAL_ATT_T *att;
        att = reinterpret_cast<SU17::GIMBAL_ATT_T *>(&temp->data[0]);
        mState.lock();
        state.abs.roll = att->roll * SU17_SCALE_FACTOR;
        state.abs.pitch = att->pitch * SU17_SCALE_FACTOR;
        state.abs.yaw = att->yaw * SU17_SCALE_FACTOR;

        updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                                  state.abs.roll, state.abs.pitch, state.abs.yaw,
                                  state.fov.x, state.fov.y, updataCaller);
        mState.unlock();
        break;

    default:
        std::cout << "Undefined frame from SU17 : ";
        for (uint16_t i = 0; i < temp->len + SU17_PAYLOAD_OFFSET + sizeof(uint16_t); i++)
        {
            printf("%02X ", ((uint8_t *)buf)[i]);
        }
        std::cout << std::endl;
        break;
    }
}

uint32_t su17GimbalDriver::calPackLen(void *pack)
{
    return ((SU17::GIMBAL_FRAME_T *)pack)->len + SU17_PAYLOAD_OFFSET + sizeof(uint16_t);
}

/**
 * The function is called every time a byte is received from the serial port. It parses the byte and
 * stores it in a buffer. When the buffer is full, it checks the CRC and if it's correct, it stores the
 * buffer in a queue
 *
 * @param uint8_t unsigned char
 *
 * @return The parser function is returning a boolean value.
 */
bool su17GimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenghte = 0;
    static uint8_t *pRx = NULL;

    switch (parserState)
    {
    case SU17::GIMBAL_SERIAL_STATE_IDEL:
        if (byte == SU17_SERIAL_HEAD)
        {
            rx.head = byte;
            parserState = SU17::GIMBAL_SERIAL_STATE_HEAD_RCV;
        }
        break;

    case SU17::GIMBAL_SERIAL_STATE_HEAD_RCV:
        if (byte == SU17_SERIAL_VERSION)
        {
            rx.version = byte;
            parserState = SU17::GIMBAL_SERIAL_STATE_VERSION_RCV;
        }
        else
        {
            rx.head = 0;
            parserState = SU17::GIMBAL_SERIAL_STATE_IDEL;
        }
        break;

    case SU17::GIMBAL_SERIAL_STATE_VERSION_RCV:
        rx.target = byte;
        parserState = SU17::GIMBAL_SERIAL_STATE_TARGET_RCV;
        break;

    case SU17::GIMBAL_SERIAL_STATE_TARGET_RCV:
        rx.source = byte;
        parserState = SU17::GIMBAL_SERIAL_STATE_SOURCE_RCV;
        break;

    case SU17::GIMBAL_SERIAL_STATE_SOURCE_RCV:
        rx.len = byte;
        parserState = SU17::GIMBAL_SERIAL_STATE_LENGHT_RCV;
        pRx = rx.data;
        payloadLenghte = byte;
        break;

    case SU17::GIMBAL_SERIAL_STATE_LENGHT_RCV:
        rx.command = byte;
        if (payloadLenghte > 0)
        {
            parserState = SU17::GIMBAL_SERIAL_STATE_DATA_RCV;
        }
        else
        {
            parserState = SU17::GIMBAL_SERIAL_STATE_CRC_RCV1;
        }

        break;

    case SU17::GIMBAL_SERIAL_STATE_DATA_RCV:
        *pRx = byte;
        payloadLenghte--;
        pRx++;
        if (payloadLenghte == 0)
        {
            parserState = SU17::GIMBAL_SERIAL_STATE_CRC_RCV1;
        }
        break;

    case SU17::GIMBAL_SERIAL_STATE_CRC_RCV1:
        rx.crc.f8[0] = byte;
        parserState = SU17::GIMBAL_SERIAL_STATE_END;
        break;

    case SU17::GIMBAL_SERIAL_STATE_END:
        rx.crc.f8[1] = byte;

        if (rx.crc.f16 == SU17::checkCrc16((uint8_t *)&rx, SU17_PAYLOAD_OFFSET + rx.len))
        {
            state = true;
            rxQueue->inCell(&rx);
        }
        else
        {
            memset(&rx, 0, sizeof(SU17::GIMBAL_FRAME_T));
        }
        parserState = SU17::GIMBAL_SERIAL_STATE_IDEL;
        break;

    default:
        parserState = SU17::GIMBAL_SERIAL_STATE_IDEL;
        break;
    }
    return state;
}