/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-06 10:27:59
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/AT10/AT10_gimbal_driver.cpp
 */
#include "AT10_gimbal_driver.h"
#include "AT10_gimbal_crc32.h"
#include "string.h"

/**
 * The function creates a new instance of the g1GimbalDriver class, which is a subclass of the
 * IamovGimbalBase class
 *
 * @param _IO The IOStreamBase object that will be used to communicate with the gimbal.
 */
AT10GimbalDriver::AT10GimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::amovGimbalBase(_IO)
{
    rxQueue = new fifoRing(sizeof(AT10::GIMBAL_EXTEND_FRAME_T), MAX_QUEUE_SIZE);
    txQueue = new fifoRing(sizeof(AT10::GIMBAL_EXTEND_FRAME_T), MAX_QUEUE_SIZE);

    stdRxQueue = new fifoRing(sizeof(AT10::GIMBAL_STD_FRAME_T), MAX_QUEUE_SIZE);
    stdTxQueue = new fifoRing(sizeof(AT10::GIMBAL_STD_FRAME_T), MAX_QUEUE_SIZE);
    parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
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
uint32_t AT10GimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;

    if (cmd > 0XFF)
    {
        AT10::GIMBAL_EXTEND_FRAME_T txTemp;
        txTemp.head = cmd;
        memcpy(txTemp.data, pPayload, payloadSize);
        payloadSize--;
        txTemp.len = payloadSize;
        if (txQueue->inCell(&txTemp))
        {
            ret = payloadSize + sizeof(uint32_t) + sizeof(uint8_t);
        }
    }
    else
    {
        AT10::GIMBAL_STD_FRAME_T txTemp;
        txTemp.head = AT10::GIMBAL_CMD_STD;
        txTemp.len = payloadSize + 3;
        txTemp.cmd = cmd;
        memcpy(txTemp.data, pPayload, payloadSize);
        txTemp.data[payloadSize] = AT10::checkXOR((uint8_t *)&txTemp.len, txTemp.len);
        if (stdTxQueue->inCell(&txTemp))
        {
            ret = payloadSize + 6;
        }
    }

    return ret;
}

void AT10GimbalDriver::convert(void *buf)
{
    AT10::GIMBAL_EXTEND_FRAME_T *temp;
    temp = reinterpret_cast<AT10::GIMBAL_EXTEND_FRAME_T *>(buf);
    switch (temp->head)
    {
    case AT10::GIMBAL_CMD_RCV_STATE:
        std::cout << "Undefined old frame from AT10\r\n";
        break;

    case AT10::GIMBAL_CMD_STD:
        AT10::GIMBAL_STD_FRAME_T *stdTemp;
        stdTemp = reinterpret_cast<AT10::GIMBAL_STD_FRAME_T *>(buf);
        switch (stdTemp->cmd)
        {
        case AT10::GIMBAL_CMD_STD_RCV_STATE:
            AT10::GIMBAL_RCV_STD_STATE_MSG_T *tempRcv;
            tempRcv = reinterpret_cast<AT10::GIMBAL_RCV_STD_STATE_MSG_T *>(((uint8_t *)buf) + AT10_STD_PAYLOAD_OFFSET);
            mState.lock();

            state.abs.roll = (amovGimbalTools::conversionBigLittle((uint16_t)(tempRcv->B1.roll & 0XFF0F)) * 0.043956043956044f) - 90.0f;
            state.abs.yaw = (int16_t)amovGimbalTools::conversionBigLittle((uint16_t)tempRcv->B1.yaw) * 0.0054931640625f;
            state.abs.pitch = (int16_t)amovGimbalTools::conversionBigLittle((uint16_t)tempRcv->B1.pitch) * 0.0054931640625f;

            state.rel.yaw = state.abs.yaw;
            state.rel.roll = state.abs.roll;
            state.rel.pitch = state.abs.pitch;

            state.fov.x = amovGimbalTools::conversionBigLittle(tempRcv->D1.fovX) * 0.1;
            state.fov.y = amovGimbalTools::conversionBigLittle(tempRcv->D1.fovY) * 0.1;
            if ((amovGimbalTools::conversionBigLittle(tempRcv->D1.camera) & 0X0003) == 0X01)
            {
                state.video = AMOV_GIMBAL_VIDEO_TAKE;
            }
            else
            {
                state.video = AMOV_GIMBAL_VIDEO_OFF;
            }
            updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                                      state.abs.roll, state.abs.pitch, state.abs.yaw,
                                      state.fov.x, state.fov.y, updataCaller);
            mState.unlock();
            break;

        case AT10::GIMBAL_CMD_STD_NOP:
            break;

        default:
            std::cout << "Undefined std frame from AT10";
            std::cout << std::endl;
            break;
        }
        break;
    default:
        printf("\r\nUndefined frame from AT10,head:%08X", temp->head);
        break;
    }
}

/**
 * It's a state machine that parses a serial stream of bytes into a struct
 *
 * @param uint8_t unsigned char
 *
 * @return A boolean value.
 */
bool AT10GimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenghte = 0;
    static uint8_t *pRx = nullptr;
    uint8_t suncheck;

    switch (parserState)
    {
    case AT10::GIMBAL_SERIAL_STATE_IDLE:
        if (byte == ((AT10::GIMBAL_CMD_RCV_STATE & 0X000000FF) >> 0))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_EXT_HEAD1;
        }
        else if (byte == ((AT10::GIMBAL_CMD_STD & 0X0000FF00) >> 8))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_STD_HAED1;
        }
        break;

    // STD msg
    case AT10::GIMBAL_SERIAL_STATE_STD_HAED1:
        if (byte == ((AT10::GIMBAL_CMD_STD & 0X00FF0000) >> 16))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_STD_HAED2;
        }
        else
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_STD_HAED2:
        if (byte == ((AT10::GIMBAL_CMD_STD & 0XFF000000) >> 24))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_STD_LEN;
        }
        else
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_STD_LEN:
        stdRx.len = byte;
        payloadLenghte = (byte & 0X3F) - 3;
        pRx = stdRx.data;
        parserState = AT10::GIMBAL_SERIAL_STATE_STD_CMD;
        break;

    case AT10::GIMBAL_SERIAL_STATE_STD_CMD:
        stdRx.cmd = byte;
        parserState = AT10::GIMBAL_SERIAL_STATE_STD_DATE;
        break;

    case AT10::GIMBAL_SERIAL_STATE_STD_DATE:
        *pRx = byte;
        pRx++;
        payloadLenghte--;
        if (payloadLenghte == 0)
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_STD_CHECK;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_STD_CHECK:
        stdRx.checkXOR = byte;
        if (AT10::checkXOR((uint8_t *)&stdRx.len, (stdRx.len & 0X3F)) == byte)
        {
            state = true;
            stdRxQueue->inCell(&stdRx);
        }
        else
        {
            memset(&stdRx, 0, sizeof(AT10::GIMBAL_STD_FRAME_T));
        }
        parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        payloadLenghte = 0;
        pRx = nullptr;

        break;

    // EXT msg
    case AT10::GIMBAL_SERIAL_STATE_EXT_HEAD1:
        if (byte == ((AT10::GIMBAL_CMD_RCV_STATE & 0X0000FF00) >> 8))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_EXT_HEAD2;
        }
        else
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_EXT_HEAD2:
        if (byte == ((AT10::GIMBAL_CMD_RCV_STATE & 0X00FF0000) >> 16))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_EXT_HEAD3;
        }
        else
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_EXT_HEAD3:
        if (byte == ((AT10::GIMBAL_CMD_RCV_STATE & 0XFF000000) >> 24))
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_EXT_DATE;
            payloadLenghte = sizeof(AT10::GIMBAL_RCV_POS_MSG_T);
            pRx = extendRx.data;
            extendRx.head = AT10::GIMBAL_CMD_RCV_STATE;
        }
        else
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_EXT_DATE:
        *pRx = byte;
        payloadLenghte--;
        pRx++;
        if (payloadLenghte == 0)
        {
            parserState = AT10::GIMBAL_SERIAL_STATE_EXT_CHECK;
        }
        break;

    case AT10::GIMBAL_SERIAL_STATE_EXT_CHECK:
        suncheck = AT10::CheckSum(extendRx.data, sizeof(AT10::GIMBAL_RCV_POS_MSG_T));
        if (byte == suncheck)
        {
            state = true;
            rxQueue->inCell(&extendRx);
        }
        else
        {
            memset(&extendRx, 0, sizeof(AT10::GIMBAL_EXTEND_FRAME_T));
        }
        parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        payloadLenghte = 0;
        pRx = nullptr;
        break;

    default:
        parserState = AT10::GIMBAL_SERIAL_STATE_IDLE;
        memset(&extendRx, 0, sizeof(AT10::GIMBAL_EXTEND_FRAME_T));
        memset(&stdRx, 0, sizeof(AT10::GIMBAL_STD_FRAME_T));
        payloadLenghte = 0;
        pRx = nullptr;
        break;
    }

    return state;
}

void AT10GimbalDriver::sendHeart(void)
{
    uint8_t temp = 0X00;
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        pack(AT10::GIMBAL_CMD_STD_HEART, &temp, sizeof(temp));
    }
}

void AT10GimbalDriver::sendStd(void)
{
    uint8_t tempBuffer[72];
    while (1)
    {
        if (!IO->isBusy() && IO->isOpen())
        {
            bool state = false;

            state = stdTxQueue->outCell(&tempBuffer);
            if (state)
            {
                IO->outPutBytes((uint8_t *)&tempBuffer + 1,
                                reinterpret_cast<AT10::GIMBAL_STD_FRAME_T *>(tempBuffer)->len + 3);
            }
        }
    }
}

void AT10GimbalDriver::stackStart(void)
{
    if (!this->IO->isOpen())
    {
        this->IO->open();
    }
    std::thread sendHeartLoop(&AT10GimbalDriver::sendHeart, this);
    std::thread sendStdLoop(&AT10GimbalDriver::sendStd, this);

    this->sendThreadHanle = sendStdLoop.native_handle();
    this->sendHreatThreadHandle = sendHeartLoop.native_handle();

    sendHeartLoop.detach();
    sendStdLoop.detach();
}

void AT10GimbalDriver::parserLoop(void)
{
    uint8_t temp[65536];
    uint32_t i = 0, getCount = 0;

    while (1)
    {
        getCount = IO->inPutBytes(temp);

        for (i = 0; i < getCount; i++)
        {
            parser(temp[i]);
        }
    }
}

void AT10GimbalDriver::getStdRxPack(void)
{
    uint8_t tempBuffer[280];
    while (1)
    {
        if (stdRxQueue->outCell(tempBuffer))
        {
            msgCustomCallback(tempBuffer, msgCaller);
            convert(tempBuffer);
        }
    }
}

void AT10GimbalDriver::getExtRxPack(void)
{
    uint8_t tempBuffer[280];
    while (1)
    {
        if (rxQueue->outCell(tempBuffer))
        {
            msgCustomCallback(tempBuffer, msgCaller);
            convert(tempBuffer);
        }
    }
}

void AT10GimbalDriver::parserStart(pAmovGimbalStateInvoke callback, void *caller)
{
    this->updateGimbalStateCallback = callback;
    this->updataCaller = caller;

    std::thread parser(&AT10GimbalDriver::parserLoop, this);
    std::thread getStdRxPackLoop(&AT10GimbalDriver::getStdRxPack, this);
    std::thread getExtRxPackLooP(&AT10GimbalDriver::getExtRxPack, this);

    this->parserThreadHanle = parser.native_handle();
    this->stackThreadHanle = getStdRxPackLoop.native_handle();
    this->extStackThreadHandle = getExtRxPackLooP.native_handle();

    parser.detach();
    getStdRxPackLoop.detach();
    getExtRxPackLooP.detach();
}

uint32_t AT10GimbalDriver::calPackLen(void *pack)
{
    return 0;
}