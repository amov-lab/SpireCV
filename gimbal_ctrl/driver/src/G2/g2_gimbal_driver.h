/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-01 10:02:24
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-13 12:29:33
 * @FilePath: \gimbal-sdk-multi-platform\src\G2\g2_gimbal_driver.h
 */
#include "../amov_gimbal.h"
#include "g2_gimbal_struct.h"
#include <mutex>
#include <malloc.h>
#include <iostream>

#ifndef __G2_DRIVER_H
#define __G2_DRIVER_H

extern "C"
{
#include "Ring_Fifo.h"
}

class g2GimbalDriver : protected amovGimbal::IamovGimbalBase
{
private:
    G2::GIMBAL_CMD_PARSER_STATE_T parserState;
    G2::GIMBAL_FRAME_T rx;
    G2::GIMBAL_FRAME_T tx;

    std::mutex rxMutex;
    uint8_t *rxBuffer;
    RING_FIFO_CB_T rxQueue;
    std::mutex txMutex;
    uint8_t *txBuffer;
    RING_FIFO_CB_T txQueue;

    uint8_t self;
    uint8_t remote;

    bool parser(IN uint8_t byte);
    void send(void);

    void convert(void *buf);
    uint32_t pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize);
    bool getRxPack(OUT void *pack);

public:
    void nodeSet(SET uint32_t _self, SET uint32_t _remote)
    {
        self = _self;
        remote = _remote;
    }

    // funtion
    uint32_t setGimabalPos(const amovGimbal::AMOV_GIMBAL_POS_T &pos);
    uint32_t setGimabalSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &speed);
    uint32_t setGimabalFollowSpeed(const amovGimbal::AMOV_GIMBAL_POS_T &followSpeed);
    uint32_t setGimabalHome(void);

    uint32_t takePic(void);
    uint32_t setVideo(const amovGimbal::AMOV_GIMBAL_VIDEO_T newState);

#ifdef AMOV_HOST
    // iap funtion (内部源码模式提供功能 lib模式下不可见)
    bool iapGetSoftInfo(std::string &info);
    bool iapGetHardInfo(std::string &info);
    bool iapJump(G2::GIMBAL_IAP_STATE_T &state);
    bool iapFlashErase(G2::GIMBAL_IAP_STATE_T &state);
    bool iapSendBlockInfo(uint32_t &startAddr, uint32_t &crc32);
    bool iapSendBlockData(uint8_t offset, uint8_t *data);
    bool iapFlashWrite(uint32_t &crc32, G2::GIMBAL_IAP_STATE_T &state);

    // 判断是否需要跳转
    bool iapJumpCheck(std::string &info) { return true; }
#endif

    static amovGimbal::IamovGimbalBase *creat(amovGimbal::IOStreamBase *_IO)
    {
        return new g2GimbalDriver(_IO);
    }

    g2GimbalDriver(amovGimbal::IOStreamBase *_IO);
    ~g2GimbalDriver()
    {
        free(rxBuffer);
        free(txBuffer);
    }
};

#endif
