/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-11-24 16:01:22
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-06 11:35:58
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amovGimbal/amov_gimbal_c.h
 */
#ifndef AMOV_GIMBAL_C_H
#define AMOV_GIMBAL_C_H

#include <stdint.h>
#include "amov_gimbal_struct.h"

extern "C"
{
    // initialization funtions
    void amovGimbalCreat(char *type, uint32_t selfId, uint32_t gimbalId, void *handle);
    void amovGimbalInBytesCallback(uint8_t *pData, uint32_t len, void *handle);
    void amovGimbalSetRcvBytes(pAmovGimbalInputBytesInvoke callbaclk, void *handle, void *caller);
    void amovGimbalSetSendBytes(pAmovGimbalOutputBytesInvoke callbaclk, void *handle, void *caller);
    void amovGimbalChangeStateCallback(pAmovGimbalStateInvoke callback, void *handle, void *caller);
    void amovGimbalSetMsgCallback(pAmovGimbalMsgInvoke callback, void *handle, void *caller);
    void amovGimbalStart(pAmovGimbalStateInvoke callback, void *handle, void *caller);

    // non-block functions
    uint32_t amovGimbalSetGimabalPos(AMOV_GIMBAL_POS_T *pos, void *handle);
    uint32_t amovGimbalSetGimabalSpeed(AMOV_GIMBAL_POS_T *speed, void *handle);
    uint32_t amovGimbalSetGimabalFollowSpeed(AMOV_GIMBAL_POS_T *followSpeed, void *handle);
    uint32_t amovGimbalSetGimabalHome(void *handle);
    uint32_t amovGimbalSetGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate, void *handle);
    uint32_t amovGimbalSetGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate, void *handle);
    uint32_t amovGimbalSetGimbalROI(AMOV_GIMBAL_ROI_T *area, void *handle);
    uint32_t amovGimbalTakePic(void *handle);
    uint32_t amovGimbalSetVideo(AMOV_GIMBAL_VIDEO_T newState, void *handle);
    uint32_t amovGimbalAttitudeCorrectionQ(AMOV_GIMBAL_QUATERNION_T *quaterion,
                                           AMOV_GIMBAL_VELOCITY_T *speed,
                                           AMOV_GIMBAL_VELOCITY_T *acc, void *extenData, void *handle);
    uint32_t amovGimbalAttitudeCorrectionE(AMOV_GIMBAL_POS_T *pos,
                                           AMOV_GIMBAL_VELOCITY_T *speed,
                                           AMOV_GIMBAL_VELOCITY_T *acc, void *extenData, void *handle);
    uint32_t amovGimbalSetGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt, void *handle);
    uint32_t amovGimbalExtensionFuntions(void *cmd, void *handle);
    void getGimabalState(AMOV_GIMBAL_STATE_T *state, void *handle);
    void getGimbalType(char *type, void *handle);

    // block functions
    bool amovGimbalSetGimbalPosBlock(AMOV_GIMBAL_POS_T *pos, void *handle);
    bool amovGimbalSetGimabalHomeBlock(void *handle);
    bool amovGimbalSetGimbalZoomBlock(float targetRate, void *handle);
    bool amovGimbalTakePicBlock(void *handle);
    bool amovGimbalCalibrationBlock(void *handle);
}

#endif
