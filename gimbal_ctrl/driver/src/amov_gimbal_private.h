/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2023-05-13 10:39:20
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:18:06
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_gimbal_private.h
 */
#ifndef __AMOV_GIMABL_PRIVATE_H
#define __AMOV_GIMABL_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <mutex>

#include "amovGimbal/amov_gimbal.h"
#include "amovGimbal/amov_gimbal_c.h"

#include "Ring_Fifo.h"
#include "amov_tool.h"
namespace amovGimbal
{
    class PamovGimbalBase
    {
    public:
        AMOV_GIMBAL_STATE_T state;
        std::mutex mState;

        // IO类
        IOStreamBase *IO = nullptr;
        // 适用于C的函数指针
        void *inBytesCaller = nullptr;
        pAmovGimbalInputBytesInvoke inBytes = nullptr;
        void *outBytesCaller = nullptr;
        pAmovGimbalOutputBytesInvoke outBytes = nullptr;

        void *updataCaller = nullptr;
        pAmovGimbalStateInvoke updateGimbalStateCallback;
        void *msgCaller = nullptr;
        pAmovGimbalMsgInvoke msgCustomCallback = idleMsgCallback;

        fifoRing *rxQueue;
        fifoRing *txQueue;

        std::thread::native_handle_type parserThreadHanle = 0;
        std::thread::native_handle_type sendThreadHanle = 0;
        std::thread::native_handle_type stackThreadHanle = 0;

        PamovGimbalBase(IOStreamBase *_IO)
        {
            IO = _IO;
        }
        virtual ~PamovGimbalBase()
        {
            if (txQueue != nullptr)
            {
                delete txQueue;
            }
            if (rxQueue != nullptr)
            {
                delete rxQueue;
            }
            // set thread kill anytime
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
            parserThreadHanle = parserThreadHanle == 0 ? 0 : pthread_cancel(parserThreadHanle);
            sendThreadHanle = sendThreadHanle == 0 ? 0 : pthread_cancel(sendThreadHanle);
            stackThreadHanle = stackThreadHanle == 0 ? 0 : pthread_cancel(stackThreadHanle);
        }
    };

    // Device interface
    class IamovGimbalBase
    {
    public:
        IamovGimbalBase() {}
        virtual ~IamovGimbalBase() {}

        // non-block functions
        virtual uint32_t setGimabalPos(const AMOV_GIMBAL_POS_T &pos);
        virtual uint32_t setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed);
        virtual uint32_t setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed);

        virtual uint32_t setGimabalHome(void);
        virtual uint32_t setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        virtual uint32_t setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
        virtual uint32_t setGimbalROI(const AMOV_GIMBAL_ROI_T area);
        virtual uint32_t takePic(void);
        virtual uint32_t setVideo(const AMOV_GIMBAL_VIDEO_T newState);
        virtual uint32_t attitudeCorrection(const AMOV_GIMBAL_QUATERNION_T &quaterion, const AMOV_GIMBAL_VELOCITY_T &speed, const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData);
        virtual uint32_t attitudeCorrection(const AMOV_GIMBAL_POS_T &pos, const AMOV_GIMBAL_VELOCITY_T &seppd, const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData);
        virtual uint32_t setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt);
        virtual uint32_t extensionFuntions(void *cmd);

        // block functions
        virtual bool setGimbalPosBlock(const AMOV_GIMBAL_POS_T &pos);
        virtual bool setGimabalHomeBlock(void);
        virtual bool setGimbalZoomBlock(float targetRate);
        virtual bool takePicBlock(void);
        virtual bool calibrationBlock(void);
    };

    class amovGimbalBase : public IamovGimbalBase, public PamovGimbalBase
    {
    public:
        virtual uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize) = 0;
        virtual bool parser(IN uint8_t byte) = 0;
        virtual void convert(void *buf) = 0;
        virtual uint32_t calPackLen(void *pack) = 0;

        virtual void send(void);
        virtual bool getRxPack(OUT void *pack);

        virtual void parserLoop(void);
        virtual void sendLoop(void);
        virtual void mainLoop(void);

        virtual void stackStart(void);
        virtual void parserStart(pAmovGimbalStateInvoke callback, void *caller);

        virtual void nodeSet(SET uint32_t _self, SET uint32_t _remote);

    public:
        amovGimbalBase(IOStreamBase *_IO);
        virtual ~amovGimbalBase();
    };
}

#endif