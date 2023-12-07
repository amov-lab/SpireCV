/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-11-24 16:00:28
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:18:34
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_gimbal_interface.cpp
 */
#include "amov_gimbal_private.h"

// must realize
void amovGimbal::gimbal::startStack(void)
{
    ((amovGimbalBase *)(this->devHandle))->stackStart();
}

void amovGimbal::gimbal::parserAuto(pAmovGimbalStateInvoke callback, void *caller)
{
    ((amovGimbalBase *)(this->devHandle))->parserStart(callback, caller);
}

void amovGimbal::gimbal::setParserCallback(pAmovGimbalStateInvoke callback, void *caller)
{
    ((amovGimbalBase *)(this->devHandle))->updateGimbalStateCallback = callback;
    ((amovGimbalBase *)(this->devHandle))->updataCaller = caller;
}

void amovGimbal::gimbal::setMsgCallback(pAmovGimbalMsgInvoke callback, void *caller)
{
    ((amovGimbalBase *)(this->devHandle))->msgCustomCallback = callback;
    ((amovGimbalBase *)(this->devHandle))->msgCaller = caller;
}

AMOV_GIMBAL_STATE_T amovGimbal::gimbal::getGimabalState(void)
{
    ((amovGimbalBase *)(this->devHandle))->mState.lock();
    AMOV_GIMBAL_STATE_T temp = ((amovGimbalBase *)(this->devHandle))->state;
    ((amovGimbalBase *)(this->devHandle))->mState.unlock();
    return temp;
}

// gimbal funtions maybe realize
uint32_t amovGimbal::gimbal::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimabalPos(pos);
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimabalSpeed(speed);
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimabalFollowSpeed(followSpeed);
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimabalHome(void)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimabalHome();
}

uint32_t amovGimbal::IamovGimbalBase::setGimabalHome(void)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimbalZoom(zoom, targetRate);
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimbalFocus(zoom, targetRate);
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGimbalROI(const AMOV_GIMBAL_ROI_T area)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimbalROI(area);
}

uint32_t amovGimbal::IamovGimbalBase::setGimbalROI(const AMOV_GIMBAL_ROI_T area)
{
    return 0;
}

uint32_t amovGimbal::gimbal::takePic(void)
{
    return ((amovGimbalBase *)(this->devHandle))->takePic();
}

uint32_t amovGimbal::IamovGimbalBase::takePic(void)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    return ((amovGimbalBase *)(this->devHandle))->setVideo(newState);
}

uint32_t amovGimbal::IamovGimbalBase::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    return 0;
}

uint32_t amovGimbal::gimbal::attitudeCorrection(const AMOV_GIMBAL_QUATERNION_T &quaterion,
                                                const AMOV_GIMBAL_VELOCITY_T &speed,
                                                const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData)
{
    return ((amovGimbalBase *)(this->devHandle))->attitudeCorrection(quaterion, speed, acc, extenData);
}

uint32_t amovGimbal::IamovGimbalBase::attitudeCorrection(const AMOV_GIMBAL_QUATERNION_T &quaterion,
                                                         const AMOV_GIMBAL_VELOCITY_T &speed,
                                                         const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData)
{
    return 0;
}

uint32_t amovGimbal::gimbal::attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                                const AMOV_GIMBAL_VELOCITY_T &speed,
                                                const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData)
{
    return ((amovGimbalBase *)(this->devHandle))->attitudeCorrection(pos, speed, acc, extenData);
}

uint32_t amovGimbal::IamovGimbalBase::attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                                         const AMOV_GIMBAL_VELOCITY_T &speed,
                                                         const AMOV_GIMBAL_VELOCITY_T &acc, void *extenData)
{
    return 0;
}

uint32_t amovGimbal::gimbal::setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt)
{
    return ((amovGimbalBase *)(this->devHandle))->setGNSSInfo(lng, lat, alt, nState, relAlt);
}

uint32_t amovGimbal::IamovGimbalBase::setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt)
{
    return 0;
}

uint32_t amovGimbal::gimbal::extensionFuntions(void *cmd)
{
    return ((amovGimbalBase *)(this->devHandle))->extensionFuntions(cmd);
}

uint32_t amovGimbal::IamovGimbalBase::extensionFuntions(void *cmd)
{
    return 0;
}

bool amovGimbal::gimbal::setGimbalPosBlock(const AMOV_GIMBAL_POS_T &pos)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimbalPosBlock(pos);
}

bool amovGimbal::IamovGimbalBase::setGimbalPosBlock(const AMOV_GIMBAL_POS_T &pos)
{
    return false;
}

bool amovGimbal::gimbal::setGimabalHomeBlock(void)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimabalHomeBlock();
}

bool amovGimbal::IamovGimbalBase::setGimabalHomeBlock(void)
{
    return false;
}

bool amovGimbal::gimbal::setGimbalZoomBlock(float targetRate)
{
    return ((amovGimbalBase *)(this->devHandle))->setGimbalZoomBlock(targetRate);
}

bool amovGimbal::IamovGimbalBase::setGimbalZoomBlock(float targetRate)
{
    return false;
}

bool amovGimbal::gimbal::takePicBlock(void)
{
    return ((amovGimbalBase *)(this->devHandle))->takePicBlock();
}

bool amovGimbal::IamovGimbalBase::takePicBlock(void)
{
    return false;
}

bool amovGimbal::gimbal::calibrationBlock(void)
{
    return ((amovGimbalBase *)(this->devHandle))->calibrationBlock();
}

bool amovGimbal::IamovGimbalBase::calibrationBlock(void)
{
    return false;
}
