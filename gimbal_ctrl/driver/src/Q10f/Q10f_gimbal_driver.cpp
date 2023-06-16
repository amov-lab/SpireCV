/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-11 17:29:58
 * @FilePath: /gimbal-sdk-multi-platform/src/Q10f/Q10f_gimbal_driver.cpp
 */
#include "Q10f_gimbal_driver.h"
#include "Q10f_gimbal_crc32.h"
#include "string.h"

/**
 * The function creates a new instance of the g1GimbalDriver class, which is a subclass of the
 * IamovGimbalBase class
 *
 * @param _IO The IOStreamBase object that will be used to communicate with the gimbal.
 */
Q10fGimbalDriver::Q10fGimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::IamovGimbalBase(_IO)
{
    memset(&rxQueue, 0, sizeof(RING_FIFO_CB_T));
    memset(&txQueue, 0, sizeof(RING_FIFO_CB_T));

    rxBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(Q10f::GIMBAL_FRAME_T));
    if (rxBuffer == NULL)
    {
        std::cout << "Receive buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }
    txBuffer = (uint8_t *)malloc(MAX_QUEUE_SIZE * sizeof(Q10f::GIMBAL_FRAME_T));
    if (txBuffer == NULL)
    {
        free(rxBuffer);
        std::cout << "Send buffer creation failed! Size : " << MAX_QUEUE_SIZE << std::endl;
        exit(1);
    }

    Ring_Fifo_init(&rxQueue, sizeof(Q10f::GIMBAL_FRAME_T), rxBuffer, MAX_QUEUE_SIZE * sizeof(Q10f::GIMBAL_FRAME_T));
    Ring_Fifo_init(&txQueue, sizeof(Q10f::GIMBAL_FRAME_T), txBuffer, MAX_QUEUE_SIZE * sizeof(Q10f::GIMBAL_FRAME_T));

    parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;

    // Initialize and enable attitude data return (50Hz)
    uint8_t cmd = 0XFF;
    pack(Q10f::GIMBAL_CMD_SET_FEEDBACK_H, &cmd, 1);
    pack(Q10f::GIMBAL_CMD_SET_FEEDBACK_L, &cmd, 1);
    cmd = 0X00;
    pack(Q10f::GIMBAL_CMD_OPEN_FEEDBACK, &cmd, 1);
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
uint32_t Q10fGimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;
    Q10f::GIMBAL_FRAME_T txTemp;

    txTemp.head = cmd;
    memcpy(txTemp.data, pPayload, payloadSize);

    if (cmd != Q10f::GIMBAL_CMD_SET_POS)
    {
        payloadSize--;
    }
    else
    {
        txTemp.data[payloadSize] = Q10f::CheckSum(pPayload, payloadSize);
    }
    txTemp.len = payloadSize;

    txMutex.lock();
    if (Ring_Fifo_in_cell(&txQueue, &txTemp))
    {
        ret = payloadSize + sizeof(uint32_t) + sizeof(uint8_t);
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
bool Q10fGimbalDriver::getRxPack(OUT void *pack)
{
    bool state = false;
    rxMutex.lock();
    state = Ring_Fifo_out_cell(&rxQueue, pack);
    rxMutex.unlock();
    return state;
}

void Q10fGimbalDriver::convert(void *buf)
{
    Q10f::GIMBAL_FRAME_T *temp;
    temp = reinterpret_cast<Q10f::GIMBAL_FRAME_T *>(buf);
    switch (temp->head)
    {
    case Q10f::GIMBAL_CMD_RCV_STATE:
        Q10f::GIMBAL_RCV_POS_MSG_T *tempPos;
        tempPos = reinterpret_cast<Q10f::GIMBAL_RCV_POS_MSG_T *>(((uint8_t *)buf) + Q10F_PAYLOAD_OFFSET);
        mState.lock();
        state.abs.yaw = tempPos->yawIMUAngle * Q10F_SCALE_FACTOR_ANGLE;
        state.abs.roll = tempPos->rollIMUAngle * Q10F_SCALE_FACTOR_ANGLE;
        state.abs.pitch = tempPos->pitchIMUAngle * Q10F_SCALE_FACTOR_ANGLE;
        state.rel.yaw = tempPos->rollStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        state.rel.roll = tempPos->rollStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        state.rel.pitch = tempPos->pitchStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        updateGimbalStateCallback(state.abs.roll, state.abs.pitch, state.abs.yaw,
                                  state.rel.roll, state.rel.pitch, state.rel.yaw,
                                  state.fov.x, state.fov.y);
        mState.unlock();

        break;
    default:
        std::cout << "Undefined frame from Q10f : ";
        for (uint16_t i = 0; i < temp->len + Q10F_PAYLOAD_OFFSET; i++)
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
void Q10fGimbalDriver::send(void)
{
    if (!IO->isBusy() && IO->isOpen())
    {
        bool state = false;
        txMutex.lock();
        state = Ring_Fifo_out_cell(&txQueue, &tx);
        txMutex.unlock();
        if (state)
        {
            IO->outPutBytes((uint8_t *)&tx, tx.len + Q10F_PAYLOAD_OFFSET + sizeof(uint8_t));
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
bool Q10fGimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenghte = 0;
    static uint8_t *pRx = NULL;
    uint8_t suncheck;

    switch (parserState)
    {
    case Q10f::GIMBAL_SERIAL_STATE_IDLE:
        if (byte == ((Q10f::GIMBAL_CMD_RCV_STATE & 0X000000FF) >> 0))
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_HEAD1;
        }
        break;

    case Q10f::GIMBAL_SERIAL_STATE_HEAD1:
        if (byte == ((Q10f::GIMBAL_CMD_RCV_STATE & 0X0000FF00) >> 8))
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_HEAD2;
        }
        else
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case Q10f::GIMBAL_SERIAL_STATE_HEAD2:
        if (byte == ((Q10f::GIMBAL_CMD_RCV_STATE & 0X00FF0000) >> 16))
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_HEAD3;
        }
        else
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case Q10f::GIMBAL_SERIAL_STATE_HEAD3:
        if (byte == ((Q10f::GIMBAL_CMD_RCV_STATE & 0XFF000000) >> 24))
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_DATE;
            payloadLenghte = sizeof(Q10f::GIMBAL_RCV_POS_MSG_T);
            pRx = rx.data;
            rx.head = Q10f::GIMBAL_CMD_RCV_STATE;
        }
        else
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case Q10f::GIMBAL_SERIAL_STATE_DATE:
        *pRx = byte;
        payloadLenghte--;
        pRx++;
        if (payloadLenghte == 0)
        {
            parserState = Q10f::GIMBAL_SERIAL_STATE_CHECK;
        }
        break;

    case Q10f::GIMBAL_SERIAL_STATE_CHECK:
        suncheck = Q10f::CheckSum(rx.data, sizeof(Q10f::GIMBAL_RCV_POS_MSG_T));
        if (byte == suncheck)
        {
            state = true;
            rxMutex.lock();
            Ring_Fifo_in_cell(&rxQueue, &rx);
            rxMutex.unlock();
        }
        else
        {
            memset(&rx, 0, sizeof(Q10f::GIMBAL_FRAME_T));
        }
        parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;
        break;

    default:
        parserState = Q10f::GIMBAL_SERIAL_STATE_IDLE;
        break;
    }

    return state;
}
