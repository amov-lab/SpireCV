/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:23:15
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/Q10f/Q10f_gimbal_driver.cpp
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
Q10fGimbalDriver::Q10fGimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::amovGimbalBase(_IO)
{
    rxQueue = new fifoRing(sizeof(Q10f::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);
    txQueue = new fifoRing(sizeof(Q10f::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);

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

    if (txQueue->inCell(&txTemp))
    {
        ret = payloadSize + sizeof(uint32_t) + sizeof(uint8_t);
    }

    return ret;
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
        state.rel.yaw = tempPos->yawStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        state.rel.roll = tempPos->rollStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        state.rel.pitch = tempPos->pitchStatorRotorAngle * Q10F_SCALE_FACTOR_SPEED;
        updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                                  state.abs.roll, state.abs.pitch, state.abs.yaw,
                                  state.fov.x, state.fov.y, updataCaller);
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

uint32_t Q10fGimbalDriver::calPackLen(void *pack)
{
    return ((Q10f::GIMBAL_FRAME_T *)pack)->len + Q10F_PAYLOAD_OFFSET + sizeof(uint8_t);
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
            rxQueue->inCell(&rx);
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
