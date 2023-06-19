/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 10:12:58
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-11 17:33:42
 * @FilePath: /gimbal-sdk-multi-platform/src/G2/g2_gimbal_driver.cpp
 */

#include "g2_gimbal_driver.h"
#include "g2_gimbal_crc.h"
#include "string.h"

/**
 * The function creates a new instance of the g2GimbalDriver class, which is a subclass of the
 * IamovGimbalBase class
 *
 * @param _IO The IOStreamBase class that is used to communicate with the gimbal.
 */
g2GimbalDriver::g2GimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::IamovGimbalBase(_IO)
{
    memset(&rxQueue, 0, sizeof(RING_FIFO_CB_T));
    memset(&txQueue, 0, sizeof(RING_FIFO_CB_T));

    rxBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(G2::GIMBAL_FRAME_T));
    if (rxBuffer == NULL)
    {
        std::cout << "Receive buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }
    txBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(G2::GIMBAL_FRAME_T));
    if (txBuffer == NULL)
    {
        free(rxBuffer);
        std::cout << "Send buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }

    Ring_Fifo_init(&rxQueue, sizeof(G2::GIMBAL_FRAME_T), rxBuffer, MAX_QUEUE_SIZE * sizeof(G2::GIMBAL_FRAME_T));
    Ring_Fifo_init(&txQueue, sizeof(G2::GIMBAL_FRAME_T), txBuffer, MAX_QUEUE_SIZE * sizeof(G2::GIMBAL_FRAME_T));

    parserState = G2::GIMBAL_SERIAL_STATE_IDEL;
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
uint32_t g2GimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;
    G2::GIMBAL_FRAME_T txTemp;

    txTemp.head = G2_SERIAL_HEAD;
    txTemp.version = G2_SERIAL_VERSION;
    txTemp.len = payloadSize;
    txTemp.command = cmd;
    txTemp.source = self;
    txTemp.target = remote;
    memcpy(txTemp.data, pPayload, payloadSize);
    txTemp.crc.f16 = G2::checkCrc16((uint8_t *)&txTemp, txTemp.len + G2_PAYLOAD_OFFSET);
    memcpy(txTemp.data + payloadSize, txTemp.crc.f8, sizeof(uint16_t));

    txMutex.lock();
    if (Ring_Fifo_in_cell(&txQueue, &txTemp))
    {
        ret = txTemp.len + G2_PAYLOAD_OFFSET + sizeof(uint16_t);
    }
    txMutex.unlock();

    return ret;
}

/**
 * > This function is used to get a packet from the receive queue
 *
 * @param void This is the type of data that will be stored in the queue.
 *
 * @return A boolean value.
 */
bool g2GimbalDriver::getRxPack(OUT void *pack)
{
    bool state = false;
    rxMutex.lock();
    state = Ring_Fifo_out_cell(&rxQueue, pack);
    rxMutex.unlock();
    return state;
}

/**
 * The function takes a pointer to a buffer, casts it to a pointer to a G2::GIMBAL_FRAME_T, and then
 * checks the command field of the frame. If the command is G2::IAP_COMMAND_BLOCK_END, it locks the
 * mutex, and then unlocks it. Otherwise, it prints out the contents of the buffer
 *
 * @param buf pointer to the data received from the gimbal
 */
void g2GimbalDriver::convert(void *buf)
{
    G2::GIMBAL_FRAME_T *temp;
    temp = reinterpret_cast<G2::GIMBAL_FRAME_T *>(buf);
    switch (temp->command)
    {
    case G2::IAP_COMMAND_BLOCK_END:
        mState.lock();
        updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                                  state.abs.roll, state.abs.pitch, state.abs.yaw,
                                  state.fov.x, state.fov.y);
        mState.unlock();
        break;

    default:
        std::cout << "Undefined frame from G2 : ";
        for (uint16_t i = 0; i < temp->len + G2_PAYLOAD_OFFSET + sizeof(uint32_t); i++)
        {
            printf("%02X ", ((uint8_t *)buf)[i]);
        }
        std::cout << std::endl;
        break;
    }
}

/**
 * If the serial port is not busy and is open, then lock the txMutex, get the next byte from the
 * txQueue, unlock the txMutex, and send the byte
 */
void g2GimbalDriver::send(void)
{
    if (!IO->isBusy() && IO->isOpen())
    {
        bool state = false;
        txMutex.lock();
        state = Ring_Fifo_out_cell(&txQueue, &tx);
        txMutex.unlock();
        if (state)
        {
            IO->outPutBytes((uint8_t *)&tx, tx.len + G2_PAYLOAD_OFFSET + sizeof(uint16_t));
        }
    }
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
bool g2GimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenghte = 0;
    static uint8_t *pRx = NULL;

    switch (parserState)
    {
    case G2::GIMBAL_SERIAL_STATE_IDEL:
        if (byte == G2_SERIAL_HEAD)
        {
            rx.head = byte;
            parserState = G2::GIMBAL_SERIAL_STATE_HEAD_RCV;
        }
        break;

    case G2::GIMBAL_SERIAL_STATE_HEAD_RCV:
        if (byte == G2_SERIAL_VERSION)
        {
            rx.version = byte;
            parserState = G2::GIMBAL_SERIAL_STATE_VERSION_RCV;
        }
        else
        {
            rx.head = 0;
            parserState = G2::GIMBAL_SERIAL_STATE_IDEL;
        }
        break;

    case G2::GIMBAL_SERIAL_STATE_VERSION_RCV:
        rx.target = byte;
        parserState = G2::GIMBAL_SERIAL_STATE_TARGET_RCV;
        break;

    case G2::GIMBAL_SERIAL_STATE_TARGET_RCV:
        rx.source = byte;
        parserState = G2::GIMBAL_SERIAL_STATE_SOURCE_RCV;
        break;

    case G2::GIMBAL_SERIAL_STATE_SOURCE_RCV:
        rx.len = byte;
        parserState = G2::GIMBAL_SERIAL_STATE_LENGHT_RCV;
        pRx = rx.data;
        payloadLenghte = byte;
        break;

    case G2::GIMBAL_SERIAL_STATE_LENGHT_RCV:
        rx.command = byte;
        parserState = G2::GIMBAL_SERIAL_STATE_DATA_RCV;
        break;

    case G2::GIMBAL_SERIAL_STATE_DATA_RCV:
        *pRx = byte;
        payloadLenghte--;
        if (payloadLenghte == 0)
        {
            parserState = G2::GIMBAL_SERIAL_STATE_CRC_RCV1;
        }
        break;

    case G2::GIMBAL_SERIAL_STATE_CRC_RCV1:
        rx.crc.f8[1] = byte;
        parserState = G2::GIMBAL_SERIAL_STATE_END;
        break;

    case G2::GIMBAL_SERIAL_STATE_END:
        rx.crc.f8[0] = byte;

        if (rx.crc.f16 == G2::checkCrc16((uint8_t *)&rx, G2_PAYLOAD_OFFSET + rx.len))
        {
            state = true;
            rxMutex.lock();
            Ring_Fifo_in_cell(&rxQueue, &rx);
            rxMutex.unlock();
        }
        else
        {
            memset(&rx, 0, sizeof(G2::GIMBAL_FRAME_T));
        }
        parserState = G2::GIMBAL_SERIAL_STATE_IDEL;
        break;

    default:
        parserState = G2::GIMBAL_SERIAL_STATE_IDEL;
        break;
    }
    return state;
}