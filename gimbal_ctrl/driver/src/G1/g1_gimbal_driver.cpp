/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 10:12:46
 * @FilePath: /gimbal-sdk-multi-platform/src/G1/g1_gimbal_driver.cpp
 */
#include "g1_gimbal_driver.h"
#include "g1_gimbal_crc32.h"
#include "string.h"

/**
 * The function creates a new instance of the g1GimbalDriver class, which is a subclass of the
 * IamovGimbalBase class
 *
 * @param _IO The IOStreamBase object that will be used to communicate with the gimbal.
 */
g1GimbalDriver::g1GimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::IamovGimbalBase(_IO)
{
    memset(&rxQueue, 0, sizeof(RING_FIFO_CB_T));
    memset(&txQueue, 0, sizeof(RING_FIFO_CB_T));

    rxBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(G1::GIMBAL_FRAME_T));
    if (rxBuffer == NULL)
    {
        std::cout << "Receive buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }
    txBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(G1::GIMBAL_FRAME_T));
    if (txBuffer == NULL)
    {
        free(rxBuffer);
        std::cout << "Send buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }

    Ring_Fifo_init(&rxQueue, sizeof(G1::GIMBAL_FRAME_T), rxBuffer, MAX_QUEUE_SIZE * sizeof(G1::GIMBAL_FRAME_T));
    Ring_Fifo_init(&txQueue, sizeof(G1::GIMBAL_FRAME_T), txBuffer, MAX_QUEUE_SIZE * sizeof(G1::GIMBAL_FRAME_T));

    parserState = G1::GIMBAL_SERIAL_STATE_IDLE;
}

/**
 * The function takes a command, a pointer to a payload, and the size of the payload. It then copies
 * the payload into the tx buffer, calculates the checksum, and then calculates the CRC32 of the
 * payload. It then copies the CRC32 into the tx buffer, and then copies the tx buffer into the txQueue
 *
 * @param uint32_t 4 bytes
 * @param pPayload pointer to the data to be sent
 * @param payloadSize the size of the payload
 *
 * @return The size of the data to be sent.
 */
uint32_t g1GimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;
    G1::GIMBAL_FRAME_T txTemp;

    txTemp.head = G1_SERIAL_HEAD;
    txTemp.version = G1_SERIAL_VERSION;
    txTemp.lenght = payloadSize;
    txTemp.cmd = cmd;
    txTemp.checksum = G1::CheckSum((unsigned char *)&txTemp.version, 3);
    memcpy(txTemp.payload, pPayload, payloadSize);
    txTemp.crc.u32 = G1::CRC32Software(txTemp.payload, payloadSize);
    memcpy(txTemp.payload + payloadSize, txTemp.crc.u8, sizeof(uint32_t));

    txMutex.lock();
    if (Ring_Fifo_in_cell(&txQueue, &txTemp))
    {
        ret = txTemp.lenght + G1_PAYLOAD_OFFSET + sizeof(uint32_t);
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
bool g1GimbalDriver::getRxPack(OUT void *pack)
{
    bool state = false;
    rxMutex.lock();
    state = Ring_Fifo_out_cell(&rxQueue, pack);
    rxMutex.unlock();
    return state;
}

void g1GimbalDriver::convert(void *buf)
{
    G1::GIMBAL_FRAME_T *temp;
    temp = reinterpret_cast<G1::GIMBAL_FRAME_T *>(buf);
    switch (temp->cmd)
    {
    case G1::GIMBAL_CMD_RCV_POS:
        G1::GIMBAL_RCV_POS_MSG_T *tempPos;
        tempPos = reinterpret_cast<G1::GIMBAL_RCV_POS_MSG_T *>(((uint8_t *)buf) + G1_PAYLOAD_OFFSET);
        mState.lock();
        state.abs.yaw = tempPos->IMU_yaw * G1_SCALE_FACTOR;
        state.abs.roll = tempPos->IMU_roll * G1_SCALE_FACTOR;
        state.abs.pitch = tempPos->IMU_pitch * G1_SCALE_FACTOR;
        state.rel.yaw = tempPos->HALL_yaw * G1_SCALE_FACTOR;
        state.rel.roll = tempPos->HALL_roll * G1_SCALE_FACTOR;
        state.rel.pitch = tempPos->HALL_pitch * G1_SCALE_FACTOR;
        updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                                  state.abs.roll, state.abs.pitch, state.abs.yaw,
                                  state.fov.x, state.fov.y);
        mState.unlock();
        break;

    default:
        std::cout << "Undefined frame from G1 : ";
        for (uint16_t i = 0; i < temp->lenght + G1_PAYLOAD_OFFSET + sizeof(uint32_t); i++)
        {
            printf("%02X ", ((uint8_t *)buf)[i]);
        }
        std::cout << std::endl;
        break;
    }
}

/**
 * The function is called by the main thread to send a command to the gimbal.
 *
 * The function first checks to see if the serial port is busy and if it is open. If it is not busy and
 * it is open, the function locks the txMutex and then checks to see if there is a command in the
 * txQueue. If there is a command in the txQueue, the function copies the command to the tx buffer and
 * then unlocks the txMutex. The function then sends the command to the gimbal.
 *
 * The txQueue is a ring buffer that holds commands that are waiting to be sent to the gimbal. The
 * txQueue is a ring buffer because the gimbal can only process one command at a time. If the gimbal is
 * busy processing a command, the command will be placed in the txQueue and sent to the gimbal when the
 * gimbal is ready to receive the command.
 */
void g1GimbalDriver::send(void)
{
    if (!IO->isBusy() && IO->isOpen())
    {
        bool state = false;
        txMutex.lock();
        state = Ring_Fifo_out_cell(&txQueue, &tx);
        txMutex.unlock();
        if (state)
        {
            IO->outPutBytes((uint8_t *)&tx, tx.lenght + G1_PAYLOAD_OFFSET + sizeof(uint32_t));
        }
    }
}

/**
 * It's a state machine that parses a serial stream of bytes into a struct
 *
 * @param uint8_t unsigned char
 *
 * @return A boolean value.
 */
bool g1GimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenghte = 0;
    static uint8_t *pRx = NULL;

    switch (parserState)
    {
    case G1::GIMBAL_SERIAL_STATE_IDLE:
        if (byte == G1_SERIAL_HEAD)
        {
            rx.head = byte;
            parserState = G1::GIMBAL_SERIAL_STATE_VERSION;
        }
        break;

    case G1::GIMBAL_SERIAL_STATE_VERSION:
        if (byte == G1_SERIAL_VERSION)
        {
            rx.version = byte;
            payloadLenghte = 0;
            parserState = G1::GIMBAL_SERIAL_STATE_LENGHT;
        }
        else
        {
            rx.head = 0;
            parserState = G1::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case G1::GIMBAL_SERIAL_STATE_LENGHT:
        payloadLenghte = byte + 4;
        rx.lenght = byte;
        parserState = G1::GIMBAL_SERIAL_STATE_CMD;
        break;

    case G1::GIMBAL_SERIAL_STATE_CMD:
        rx.cmd = byte;
        parserState = G1::GIMBAL_SERIAL_STATE_CHECK;
        break;

    case G1::GIMBAL_SERIAL_STATE_CHECK:
        rx.checksum = byte;
        if (G1::CheckSum((unsigned char *)&rx.version, 3) == byte)
        {

            parserState = G1::GIMBAL_SERIAL_STATE_PAYLOAD;
            pRx = rx.payload;
        }
        else
        {
            memset(&rx, 0, 5);
            parserState = G1::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case G1::GIMBAL_SERIAL_STATE_PAYLOAD:
        *pRx = byte;
        payloadLenghte--;
        pRx++;
        if (payloadLenghte <= 0)
        {
            if (*((uint32_t *)(pRx - sizeof(uint32_t))) == G1::CRC32Software(rx.payload, rx.lenght))
            {
                state = true;
                rxMutex.lock();
                Ring_Fifo_in_cell(&rxQueue, &rx);
                rxMutex.unlock();
            }
            else
            {
                memset(&rx, 0, sizeof(G1::GIMBAL_FRAME_T));
            }
            parserState = G1::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    default:
        parserState = G1::GIMBAL_SERIAL_STATE_IDLE;
        break;
    }
    return state;
}
