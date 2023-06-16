/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-28 11:54:11
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-11 18:13:25
 * @FilePath: /gimbal-sdk-multi-platform/src/amov_gimabl.cpp
 */

#include "amov_gimbal.h"
#include "g1_gimbal_driver.h"
#include "g2_gimbal_driver.h"
#include "Q10f_gimbal_driver.h"

#include <iostream>
#include <thread>
#include <map>
#include <iterator>

#define MAX_PACK_SIZE 280
typedef enum
{
    AMOV_GIMBAL_TYPE_NULL,
    AMOV_GIMBAL_TYPE_G1 = 1,
    AMOV_GIMBAL_TYPE_G2,
    AMOV_GIMBAL_TYPE_Q10,
} AMOV_GIMBAL_TYPE_T;

namespace amovGimbal
{
    typedef amovGimbal::IamovGimbalBase *(*createCallback)(amovGimbal::IOStreamBase *_IO);
    typedef std::map<std::string, createCallback> callbackMap;
    std::map<std::string, AMOV_GIMBAL_TYPE_T> amovGimbalTypeList =
        {
            {"G1", AMOV_GIMBAL_TYPE_G1},
            {"G2", AMOV_GIMBAL_TYPE_G2},
            {"Q10f", AMOV_GIMBAL_TYPE_Q10}};

    callbackMap amovGimbals =
        {
            {"G1", g1GimbalDriver::creat},
            {"G2", g2GimbalDriver::creat},
            {"Q10f", Q10fGimbalDriver::creat}};
}

/* The amovGimbalCreator class is a factory class that creates an instance of the amovGimbal class */
// Factory used to create the gimbal instance
class amovGimbalCreator
{
public:
    static amovGimbal::IamovGimbalBase *createAmovGimbal(const std::string &type, amovGimbal::IOStreamBase *_IO)
    {
        amovGimbal::callbackMap::iterator temp = amovGimbal::amovGimbals.find(type);

        if (temp != amovGimbal::amovGimbals.end())
        {
            return (temp->second)(_IO);
        }
        std::cout << type << " is Unsupported device type!" << std::endl;
        return NULL;
    }

private:
    amovGimbalCreator()
    {
    }
    static amovGimbalCreator *pInstance;
    static amovGimbalCreator *getInstance()
    {
        if (pInstance == NULL)
        {
            pInstance = new amovGimbalCreator();
        }
        return pInstance;
    }

    ~amovGimbalCreator();
};

/**
 * "If the input byte is available, then parse it."
 *
 * The function is a loop that runs forever. It calls the IO->inPutByte() function to get a byte from
 * the serial port. If the byte is available, then it calls the parser() function to parse the byte
 */
void amovGimbal::IamovGimbalBase::parserLoop(void)
{
    uint8_t temp;

    while (1)
    {
        if (IO->inPutByte(&temp))
        {
            parser(temp);
        }
    }
}

void amovGimbal::IamovGimbalBase::sendLoop(void)
{
    while (1)
    {
        send();
    }
}

void amovGimbal::IamovGimbalBase::mainLoop(void)
{
    uint8_t tempBuffer[MAX_PACK_SIZE];

    while (1)
    {
        if (getRxPack(tempBuffer))
        {
            convert(tempBuffer);
        }
    }
}

/**
 * It starts two threads, one for reading data from the serial port and one for sending data to the
 * serial port
 */
void amovGimbal::IamovGimbalBase::startStack(void)
{
    if (!IO->isOpen())
    {
        IO->open();
    }

    std::thread mainLoop(&IamovGimbalBase::parserLoop, this);
    std::thread sendLoop(&IamovGimbalBase::sendLoop, this);
    mainLoop.detach();
    sendLoop.detach();
}

/**
 * The function creates a thread that runs the mainLoop function
 */
void amovGimbal::IamovGimbalBase::parserAuto(pStateInvoke callback)
{
    this->updateGimbalStateCallback = callback;
    std::thread mainLoop(&IamovGimbalBase::mainLoop, this);
    mainLoop.detach();
}

amovGimbal::AMOV_GIMBAL_STATE_T amovGimbal::IamovGimbalBase::getGimabalState(void)
{
    mState.lock();
    AMOV_GIMBAL_STATE_T temp = state;
    mState.unlock();
    return temp;
}

amovGimbal::IamovGimbalBase::~IamovGimbalBase()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    IO->close();
}

/**
 * Default implementation of interface functions, not pure virtual functions for ease of extension.
 */
void amovGimbal::IamovGimbalBase::nodeSet(SET uint32_t _self, SET uint32_t _remote)
{
    return;
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalPos(const amovGimbal::AMOV_GIMBAL_POS_T &pos)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &speed)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalFollowSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &followSpeed)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalHome(void)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalZoom(amovGimbal::AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalFocus(amovGimbal::AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalROI(const amovGimbal::AMOV_GIMBAL_ROI_T area)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::takePic(void)
{
    return 0;
}

uint32_t amovGimbal::IamovGimbalBase::setVideo(const amovGimbal::AMOV_GIMBAL_VIDEO_T newState)
{
    return 0;
}

/**
 * The function creates a new gimbal object, which is a pointer to a new amovGimbal object, which is a
 * pointer to a new Gimbal object, which is a pointer to a new IOStreamBase object
 *
 * @param type the type of the device, which is the same as the name of the class
 * @param _IO The IOStreamBase object that is used to communicate with the device.
 * @param _self the node ID of the device
 * @param _remote the node ID of the remote device
 */
amovGimbal::gimbal::gimbal(const std::string &type, IOStreamBase *_IO,
                           uint32_t _self, uint32_t _remote)
{
    typeName = type;
    IO = _IO;

    dev = amovGimbalCreator::createAmovGimbal(typeName, IO);

    dev->nodeSet(_self, _remote);
}

amovGimbal::gimbal::~gimbal()
{
    // 先干掉请求线程
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    delete dev;
}
