/*
 * @Description:    External interface of amov gimbals
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:34:26
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:37:09
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amovGimbal/amov_gimbal.h
 */

#ifndef AMOV_GIMBAL_H
#define AMOV_GIMBAL_H

#include <stdint.h>
#include <stdbool.h>
#include <iostream>

#include "amov_gimbal_struct.h"

namespace amovGimbal
{
#define IN
#define OUT
#define SET

#ifndef MAX_QUEUE_SIZE
#define MAX_QUEUE_SIZE 100
#endif

    static inline void idleCallback(double frameAngleRoll, double frameAnglePitch, double frameAngleYaw,
                                    double imuAngleRoll, double imuAnglePitch, double imuAngleYaw,
                                    double fovX, double fovY, void *caller)
    {
    }
    static inline void idleMsgCallback(void *msg, void *caller)
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
        virtual uint32_t inPutBytes(IN uint8_t *byte) = 0;
        virtual uint32_t outPutBytes(IN uint8_t *byte, uint32_t lenght) = 0;
    };

    class gimbal
    {
    private:
        std::string typeName;
        // Instantiated device handle
        void *devHandle;

    public:
        static void inBytesCallback(uint8_t *pData, uint32_t len, gimbal *handle);
        // Protocol stack function items
        void startStack(void);
        void parserAuto(pAmovGimbalStateInvoke callback = idleCallback, void *caller = nullptr);
        void setParserCallback(pAmovGimbalStateInvoke callback, void *caller = nullptr);
        void setMsgCallback(pAmovGimbalMsgInvoke callback, void *caller = nullptr);
        void setRcvBytes(pAmovGimbalInputBytesInvoke callbaclk, void *caller = nullptr);
        void setSendBytes(pAmovGimbalOutputBytesInvoke callbaclk, void *caller = nullptr);
        AMOV_GIMBAL_STATE_T getGimabalState(void);

        // non-block functions
        uint32_t setGimabalPos(const AMOV_GIMBAL_POS_T &pos);
        uint32_t setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed);
        uint32_t setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed);

        uint32_t setGimabalHome(void);
        uint32_t setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        uint32_t setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        uint32_t setGimbalROI(const AMOV_GIMBAL_ROI_T area);
        uint32_t takePic(void);
        uint32_t setVideo(const AMOV_GIMBAL_VIDEO_T newState);
        uint32_t attitudeCorrection(const AMOV_GIMBAL_QUATERNION_T &quaterion,
                                    const AMOV_GIMBAL_VELOCITY_T &speed,
                                    const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData);
        uint32_t attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                    const AMOV_GIMBAL_VELOCITY_T &seppd,
                                    const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData);
        uint32_t setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt);
        uint32_t extensionFuntions(void *cmd);

        // block functions
        bool setGimbalPosBlock(const AMOV_GIMBAL_POS_T &pos);
        bool setGimabalHomeBlock(void);
        bool setGimbalZoomBlock(float targetRate);
        bool takePicBlock(void);
        bool calibrationBlock(void);

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