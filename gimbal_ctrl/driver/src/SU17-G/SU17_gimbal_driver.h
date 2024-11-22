/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 10:02:24
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 18:03:11
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/SU17-G/SU17_gimbal_driver.h
 */
#ifndef __SU17_G_DRIVER_H
#define __SU17_G_DRIVER_H
#include "../amov_gimbal_private.h"
#include "SU17_gimbal_struct.h"
#include <mutex>
#include <malloc.h>
#include <iostream>
#include <thread>

class su17GimbalDriver : protected amovGimbal::amovGimbalBase
{
private:
    SU17::GIMBAL_CMD_PARSER_STATE_T parserState;
    SU17::GIMBAL_FRAME_T rx;

    uint8_t self;
    uint8_t remote;

    bool parser(IN uint8_t byte);
    void convert(void *buf);
    uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize);
    uint32_t calPackLen(void *pack);

    void heart(void);
    std::thread::native_handle_type heartThreadHandle;

public:
    void nodeSet(SET uint32_t _self, SET uint32_t _remote)
    {
        self = _self;
        remote = _remote;
    }

    // funtion
    uint32_t setGimabalPos(const AMOV_GIMBAL_POS_T &pos);
    uint32_t setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed);
    // uint32_t setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed);
    // uint32_t setGimabalHome(void);

    uint32_t takePic(void);
    uint32_t setVideo(const AMOV_GIMBAL_VIDEO_T newState);

    static amovGimbal::amovGimbalBase *creat(amovGimbal::IOStreamBase *_IO)
    {
        return new su17GimbalDriver(_IO);
    }

    su17GimbalDriver(amovGimbal::IOStreamBase *_IO);

    ~su17GimbalDriver()
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
        heartThreadHandle = heartThreadHandle == 0 ? 0 : pthread_cancel(heartThreadHandle);
    }
};

#endif
