/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-11-24 15:55:37
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:19:19
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_gimbal_realize.cpp
 */

#include "amov_gimbal_private.h"

#include <thread>

#define MAX_PACK_SIZE 280

/**
 * This is a constructor for the amovGimbalBase class that initializes its parent classes with an
 * IOStreamBase object.
 *
 * @param _IO _IO is a pointer to an object of type amovGimbal::IOStreamBase, which is the base class
 * for input/output streams used by the amovGimbal class. This parameter is passed to the constructor
 * of amovGimbalBase, which is a derived class of I
 */
amovGimbal::amovGimbalBase::amovGimbalBase(amovGimbal::IOStreamBase *_IO) : amovGimbal::IamovGimbalBase(), amovGimbal::PamovGimbalBase(_IO)
{
}

/**
 * The function is a destructor that sleeps for 50 milliseconds and closes an IO object.
 */
amovGimbal::amovGimbalBase::~amovGimbalBase()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    IO->close();
}

/**
 * This function retrieves a packet from a ring buffer queue and returns a boolean value indicating
 * whether the operation was successful or not.
 *
 * @param void void is a keyword in C++ that represents the absence of a type. In this function, it is
 * used to indicate that the function does not return any value.
 *
 * @return a boolean value, which indicates whether or not a data packet was successfully retrieved
 * from a ring buffer queue.
 */
bool amovGimbal::amovGimbalBase::getRxPack(OUT void *pack)
{
    bool state = false;
    state = rxQueue->outCell(pack);
    return state;
}

/**
 * This function sends data from a buffer to an output device if it is not busy and open.
 */
void amovGimbal::amovGimbalBase::send(void)
{
    uint8_t tempBuffer[MAX_PACK_SIZE];

    if (IO == nullptr)
    {
        while (1)
        {
            if (txQueue->outCell(&tempBuffer))
            {
                this->outBytes((uint8_t *)&tempBuffer, calPackLen(tempBuffer), outBytesCaller);
            }
        }
    }
    else
    {
        while (1)
        {
            if (!IO->isBusy() && IO->isOpen())
            {
                if (txQueue->outCell(&tempBuffer))
                {
                    IO->outPutBytes((uint8_t *)&tempBuffer, calPackLen(tempBuffer));
                }
            }
        }
    }
}

/**
 * "If the input byte is available, then parse it."
 *
 * The function is a loop that runs forever. It calls the IO->inPutByte() function to get a byte from
 * the serial port. If the byte is available, then it calls the parser() function to parse the byte
 */
void amovGimbal::amovGimbalBase::parserLoop(void)
{
    uint8_t temp[65536];
    uint32_t i = 0, getCount = 0;
    if (IO == nullptr)
    {
        while (1)
        {
            getCount = inBytes(temp, inBytesCaller);

            for (i = 0; i < getCount; i++)
            {
                parser(temp[i]);
            }
        }
    }
    else
    {
        while (1)
        {
            getCount = IO->inPutBytes(temp);

            for (i = 0; i < getCount; i++)
            {
                parser(temp[i]);
            }
        }
    }
}

void amovGimbal::amovGimbalBase::sendLoop(void)
{
    send();
}

void amovGimbal::amovGimbalBase::mainLoop(void)
{
    uint8_t tempBuffer[MAX_PACK_SIZE];

    while (1)
    {
        if (getRxPack(tempBuffer))
        {
            msgCustomCallback(tempBuffer, msgCaller);
            convert(tempBuffer);
        }
    }
}

void amovGimbal::amovGimbalBase::stackStart(void)
{
    if (!this->IO->isOpen() && this->IO != nullptr)
    {
        this->IO->open();
    }

    // 当且仅当需要库主动查询时才启用解析器线程
    if (inBytes != nullptr || this->IO != nullptr)
    {
        std::thread parserLoop(&amovGimbalBase::parserLoop, this);
        this->parserThreadHanle = parserLoop.native_handle();
        parserLoop.detach();
    }

    std::thread sendLoop(&amovGimbalBase::sendLoop, this);
    this->sendThreadHanle = sendLoop.native_handle();
    sendLoop.detach();
}

void amovGimbal::amovGimbalBase::parserStart(pAmovGimbalStateInvoke callback, void *caller)
{
    this->updateGimbalStateCallback = callback;
    this->updataCaller = caller;

    std::thread mainLoop(&amovGimbalBase::mainLoop, this);

    this->stackThreadHanle = mainLoop.native_handle();

    mainLoop.detach();
}

void amovGimbal::amovGimbalBase::nodeSet(SET uint32_t _self, SET uint32_t _remote)
{
    return;
}

void amovGimbal::gimbal::setRcvBytes(pAmovGimbalInputBytesInvoke callbaclk, void *caller)
{
    ((amovGimbal::amovGimbalBase *)(this->devHandle))->inBytes = callbaclk;
    ((amovGimbal::amovGimbalBase *)(this->devHandle))->inBytesCaller = caller;
}

void amovGimbal::gimbal::setSendBytes(pAmovGimbalOutputBytesInvoke callbaclk, void *caller)
{
    ((amovGimbal::amovGimbalBase *)(this->devHandle))->outBytes = callbaclk;
    ((amovGimbal::amovGimbalBase *)(this->devHandle))->outBytesCaller = caller;
}

void amovGimbal::gimbal::inBytesCallback(uint8_t *pData, uint32_t len, gimbal *handle)
{
    uint32_t i = 0;
    for (i = 0; i < len; i++)
    {
        ((amovGimbalBase *)((handle)->devHandle))->parser(pData[i]);
    }
}
