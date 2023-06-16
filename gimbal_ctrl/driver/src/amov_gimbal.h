/*
 * @Description:    External interface of amov gimbals
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:34:26
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 11:42:05
 * @FilePath: /spirecv-gimbal-sdk/gimbal_ctrl/driver/src/amov_gimbal.h
 */

#ifndef AMOV_GIMBAL_H
#define AMOV_GIMBAL_H

#include <stdint.h>
#include <stdbool.h>
#include <iostream>

#include <thread>
#include <unistd.h>
#include <mutex>

#include "amov_gimbal_struct.h"

#define MAX_QUEUE_SIZE 50

namespace amovGimbal
{
#define IN
#define OUT
#define SET

    static inline void idleCallback(double &frameAngleRoll, double &frameAnglePitch, double &frameAngleYaw,
                                    double &imuAngleRoll, double &imuAnglePitch, double &imuAngleYaw,
                                    double &fovX, double &fovY)
    {
    }

    // Control data input and output
    class IOStreamBase
    {
    public:
        IOStreamBase() {}
        virtual ~IOStreamBase() {}

        virtual bool open() = 0;
        virtual bool close() = 0;
        virtual bool isOpen() = 0;
        virtual bool isBusy() = 0;
        // These two functions need to be thread-safe
        virtual bool inPutByte(IN uint8_t *byte) = 0;
        virtual uint32_t outPutBytes(IN uint8_t *byte, uint32_t lenght) = 0;
    };

    class IamovGimbalBase
    {
    protected:
        AMOV_GIMBAL_STATE_T state;
        std::mutex mState;
        IOStreamBase *IO;
        pStateInvoke updateGimbalStateCallback;

        virtual bool parser(IN uint8_t byte) = 0;
        virtual void send(void) = 0;
        virtual void convert(void *buf) = 0;
        virtual uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize) = 0;
        virtual bool getRxPack(OUT void *pack) = 0;

        void parserLoop(void);
        void sendLoop(void);
        void mainLoop(void);

    public:
        IamovGimbalBase(SET IOStreamBase *_IO)
        {
            IO = _IO;
        }
        virtual ~IamovGimbalBase();

        void setParserCallback(pStateInvoke callback)
        {
            this->updateGimbalStateCallback = callback;
        }

        // Protocol stack function items
        virtual void startStack(void);
        virtual void parserAuto(pStateInvoke callback = idleCallback);
        virtual void nodeSet(SET uint32_t _self, SET uint32_t _remote);

        // functions
        virtual AMOV_GIMBAL_STATE_T getGimabalState(void);
        virtual uint32_t setGimabalPos(const AMOV_GIMBAL_POS_T &pos);
        virtual uint32_t setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed);
        virtual uint32_t setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed);
        virtual uint32_t setGimabalHome(void);
        virtual uint32_t setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        virtual uint32_t setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        virtual uint32_t setGimbalROI(const AMOV_GIMBAL_ROI_T area);
        virtual uint32_t takePic(void);
        virtual uint32_t setVideo(const AMOV_GIMBAL_VIDEO_T newState);
    };

    class gimbal
    {
    private:
        std::string typeName;
        IOStreamBase *IO;

    public:
        IamovGimbalBase *dev;
        std::string name()
        {
            return typeName;
        }
        gimbal(const std::string &type, IOStreamBase *_IO,
               uint32_t _self = 0x02, uint32_t _remote = 0X80);
        ~gimbal();
    };
}
#endif