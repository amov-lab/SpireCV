/*
 * @Description:    G1吊舱的驱动文件
 * @Author: L LC @amov
 * @Date: 2022-10-28 12:24:21
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-13 12:29:17
 * @FilePath: \gimbal-sdk-multi-platform\src\G1\g1_gimbal_driver.h
 */
#include "../amov_gimbal.h"
#include "g1_gimbal_struct.h"
#include <mutex>
#include <malloc.h>
#include <iostream>

#ifndef __G1_DRIVER_H
#define __G1_DRIVER_H

extern "C"
{
#include "Ring_Fifo.h"
}

class g1GimbalDriver : protected amovGimbal::IamovGimbalBase
{
private:
    G1::GIMBAL_CMD_PARSER_STATE_T parserState;
    G1::GIMBAL_FRAME_T rx;
    G1::GIMBAL_FRAME_T tx;

    std::mutex rxMutex;
    uint8_t *rxBuffer;
    RING_FIFO_CB_T rxQueue;
    std::mutex txMutex;
    uint8_t *txBuffer;
    RING_FIFO_CB_T txQueue;

    bool parser(IN uint8_t byte);
    void send(void);

    void convert(void *buf);
    uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize);
    bool getRxPack(OUT void *pack);

public:
    // funtions
    uint32_t setGimabalPos(const amovGimbal::AMOV_GIMBAL_POS_T &pos);
    uint32_t setGimabalSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &speed);
    uint32_t setGimabalFollowSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &followSpeed);
    uint32_t setGimabalHome(void);

    uint32_t takePic(void);
    uint32_t setVideo(const amovGimbal::AMOV_GIMBAL_VIDEO_T newState);

    // builds
    static amovGimbal::IamovGimbalBase *creat(amovGimbal::IOStreamBase *_IO)
    {
        return new g1GimbalDriver(_IO);
    }

    g1GimbalDriver(amovGimbal::IOStreamBase *_IO);
    ~g1GimbalDriver()
    {
        free(rxBuffer);
        free(txBuffer);
    }
};

#endif
