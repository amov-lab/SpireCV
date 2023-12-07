/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-11-27 12:28:32
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-06 11:36:30
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_gimbal_interface_c.cpp
 */
#include "amov_gimbal_private.h"
#include <string>

void amovGimbalSetRcvBytes(pAmovGimbalInputBytesInvoke callbaclk, void *handle, void *caller)
{
    ((amovGimbal::gimbal *)handle)->setRcvBytes(callbaclk, caller);
}

void amovGimbalSetSendBytes(pAmovGimbalOutputBytesInvoke callbaclk, void *handle, void *caller)
{
    ((amovGimbal::gimbal *)handle)->setSendBytes(callbaclk, caller);
}

void amovGimbalInBytesCallback(uint8_t *pData, uint32_t len, void *handle)
{
    amovGimbal::gimbal::inBytesCallback(pData, len, (amovGimbal::gimbal *)handle);
}

void amovGimbalCreat(char *type, uint32_t selfId, uint32_t gimbalId, void *handle)
{
    std::string strType = type;
    handle = new amovGimbal::gimbal(strType, nullptr, selfId, gimbalId);
}

void amovGimbalStart(pAmovGimbalStateInvoke callback, void *handle, void *caller)
{
    ((amovGimbal::gimbal *)handle)->startStack();
    ((amovGimbal::gimbal *)handle)->parserAuto(callback, caller);
}

void amovGimbalChangeStateCallback(pAmovGimbalStateInvoke callback, void *handle, void *caller)
{
    ((amovGimbal::gimbal *)handle)->setParserCallback(callback, caller);
}

void amovGimbalSetMsgCallback(pAmovGimbalMsgInvoke callback, void *handle, void *caller)
{
    ((amovGimbal::gimbal *)handle)->setMsgCallback(callback, caller);
}

uint32_t amovGimbalSetGimabalPos(AMOV_GIMBAL_POS_T *pos, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimabalPos(*pos);
}

uint32_t amovGimbalSetGimabalSpeed(AMOV_GIMBAL_POS_T *speed, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimabalSpeed(*speed);
}

uint32_t amovGimbalSetGimabalFollowSpeed(AMOV_GIMBAL_POS_T *followSpeed, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimabalFollowSpeed(*followSpeed);
}

uint32_t amovGimbalSetGimabalHome(void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimabalHome();
}

uint32_t amovGimbalSetGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimbalZoom(zoom, targetRate);
}

uint32_t amovGimbalSetGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimbalFocus(zoom, targetRate);
}

uint32_t amovGimbalSetGimbalROI(AMOV_GIMBAL_ROI_T *area, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimbalROI(*area);
}

uint32_t amovGimbalTakePic(void *handle)
{
    return ((amovGimbal::gimbal *)handle)->takePic();
}

uint32_t amovGimbalSetVideo(AMOV_GIMBAL_VIDEO_T newState, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setVideo(newState);
}

uint32_t amovGimbalAttitudeCorrectionQ(AMOV_GIMBAL_QUATERNION_T *quaterion,
                                       AMOV_GIMBAL_VELOCITY_T *speed,
                                       AMOV_GIMBAL_VELOCITY_T *acc, void *extenData, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->attitudeCorrection(*quaterion, *speed, *acc, extenData);
}

uint32_t amovGimbalAttitudeCorrectionE(AMOV_GIMBAL_POS_T *pos,
                                       AMOV_GIMBAL_VELOCITY_T *speed,
                                       AMOV_GIMBAL_VELOCITY_T *acc, void *extenData, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->attitudeCorrection(*pos, *speed, *acc, extenData);
}

uint32_t amovGimbalSetGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGNSSInfo(lng, lat, alt, nState, relAlt);
}

uint32_t amovGimbalExtensionFuntions(void *cmd, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->extensionFuntions(cmd);
}

void getGimabalState(AMOV_GIMBAL_STATE_T *state, void *handle)
{
    *state = ((amovGimbal::gimbal *)handle)->getGimabalState();
}

void getGimbalType(char *type, void *handle)
{
    std::string temp = ((amovGimbal::gimbal *)handle)->name();
    temp.copy(type, temp.size(), 0);
}

bool amovGimbalSetGimbalPosBlock(AMOV_GIMBAL_POS_T *pos, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimbalPosBlock(*pos);
}

bool amovGimbalSetGimabalHomeBlock(void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimabalHomeBlock();
}

bool amovGimbalSetGimbalZoomBlock(float targetRate, void *handle)
{
    return ((amovGimbal::gimbal *)handle)->setGimbalZoomBlock(targetRate);
}

bool amovGimbalTakePicBlock(void *handle)
{
    return ((amovGimbal::gimbal *)handle)->takePicBlock();
}

bool amovGimbalCalibrationBlock(void *handle)
{
    return ((amovGimbal::gimbal *)handle)->calibrationBlock();
}