
/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-10-20 16:08:13
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-06 10:27:05
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/GX40/GX40_gimbal_driver.h
 */
#ifndef __GX40_DRIVER_H
#define __GX40_DRIVER_H
#include "../amov_gimbal_private.h"
#include "GX40_gimbal_struct.h"
#include <mutex>
#include <malloc.h>
#include <iostream>
#include <chrono>
#include <time.h>

class GX40GimbalDriver : public amovGimbal::amovGimbalBase
{
    GX40::GIMBAL_FRAME_PARSER_STATE_T parserState;
    GX40::GIMBAL_FRAME_T rx;

    std::chrono::milliseconds upDataTs;
    std::mutex carrierStateMutex;

    int16_t targetPos[3];

    AMOV_GIMBAL_POS_T carrierPos;
    AMOV_GIMBAL_VELOCITY_T carrierSpeed;
    AMOV_GIMBAL_VELOCITY_T carrierAcc;
    GX40::GIMBAL_SECONDARY_MASTER_FRAME_T carrierGNSS;

    std::thread::native_handle_type nopSendThreadHandle;
    void nopSend(void);
    void parserStart(pAmovGimbalStateInvoke callback, void *caller);

public:
    uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize);
    bool parser(IN uint8_t byte);
    void convert(void *buf);
    uint32_t calPackLen(void *pack);

    // funtions
    uint32_t setGimabalPos(const AMOV_GIMBAL_POS_T &pos);
    uint32_t setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed);
    uint32_t setGimabalHome(void);

    uint32_t setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);
    uint32_t setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate = 0);

    uint32_t takePic(void);
    uint32_t setVideo(const AMOV_GIMBAL_VIDEO_T newState);
    uint32_t attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                const AMOV_GIMBAL_VELOCITY_T &seppd,
                                const AMOV_GIMBAL_VELOCITY_T &acc,
                                void *extenData);

    uint32_t setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt);
    uint32_t extensionFuntions(void *cmd);
    static amovGimbal::amovGimbalBase *creat(amovGimbal::IOStreamBase *_IO)
    {
        return new GX40GimbalDriver(_IO);
    }
    GX40GimbalDriver(amovGimbal::IOStreamBase *_IO);
    ~GX40GimbalDriver()
    {
        if (txQueue != nullptr)
        {
            delete txQueue;
        }
        if (rxQueue != nullptr)
        {
            delete rxQueue;
        }

        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        parserThreadHanle = parserThreadHanle == 0 ? 0 : pthread_cancel(parserThreadHanle);
        sendThreadHanle = sendThreadHanle == 0 ? 0 : pthread_cancel(sendThreadHanle);
        stackThreadHanle = stackThreadHanle == 0 ? 0 : pthread_cancel(stackThreadHanle);
        nopSendThreadHandle = nopSendThreadHandle == 0 ? 0 : pthread_cancel(nopSendThreadHandle);
    }
};

#endif
