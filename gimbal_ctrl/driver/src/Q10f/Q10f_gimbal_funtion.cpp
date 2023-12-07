/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-02 10:00:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:27:39
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/Q10f/Q10f_gimbal_funtion.cpp
 */
#include "Q10f_gimbal_driver.h"
#include "Q10f_gimbal_crc32.h"
#include "string.h"

/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t Q10fGimbalDriver::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    Q10f::GIMBAL_SET_POS_MSG_T temp;
    temp.modeR = Q10f::GIMBAL_CMD_POS_MODE_ANGLE_SPEED;
    temp.modeP = Q10f::GIMBAL_CMD_POS_MODE_ANGLE_SPEED;
    temp.modeY = Q10f::GIMBAL_CMD_POS_MODE_ANGLE_SPEED;

    temp.angleP = pos.pitch / Q10F_SCALE_FACTOR_ANGLE;
    temp.angleR = pos.roll / Q10F_SCALE_FACTOR_ANGLE;
    temp.angleY = pos.yaw / Q10F_SCALE_FACTOR_ANGLE;
    temp.speedP = state.maxFollow.pitch;
    temp.speedR = state.maxFollow.roll;
    temp.speedY = state.maxFollow.yaw;
    return pack(Q10f::GIMBAL_CMD_SET_POS, reinterpret_cast<uint8_t *>(&temp), sizeof(Q10f::GIMBAL_SET_POS_MSG_T));
}

/**
 * It takes a struct of type amovGimbal::AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t Q10fGimbalDriver::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    Q10f::GIMBAL_SET_POS_MSG_T temp;
    temp.modeR = Q10f::GIMBAL_CMD_POS_MODE_SPEED;
    temp.modeP = Q10f::GIMBAL_CMD_POS_MODE_SPEED;
    temp.modeY = Q10f::GIMBAL_CMD_POS_MODE_SPEED;

    temp.angleP = 0;
    temp.angleR = 0;
    temp.angleY = 0;
    temp.speedP = speed.pitch / 0.1220740379f;
    temp.speedR = speed.roll / 0.1220740379f;
    temp.speedY = speed.yaw / 0.1220740379f;
    return pack(Q10f::GIMBAL_CMD_SET_POS, reinterpret_cast<uint8_t *>(&temp), sizeof(Q10f::GIMBAL_SET_POS_MSG_T));
}

/**
 * This function sets the gimbal's follow speed
 *
 * @param followSpeed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t Q10fGimbalDriver::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
{
    state.maxFollow.pitch = followSpeed.pitch / 0.1220740379f;
    state.maxFollow.roll = followSpeed.roll / 0.1220740379f;
    state.maxFollow.yaw = followSpeed.yaw / 0.1220740379f;
    return 0;
}

/**
 * This function sets the gimbal to its home position
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t Q10fGimbalDriver::setGimabalHome(void)
{
    // amovGimbal::AMOV_GIMBAL_POS_T home;
    // home.pitch = 0;
    // home.roll = 0;
    // home.yaw = 0;
    // return setGimabalPos(home);

    const static uint8_t cmd[5] = {0X00, 0X00, 0X03, 0X03, 0XFF};
    return pack(Q10f::GIMBAL_CMD_HOME, (uint8_t *)cmd, sizeof(cmd));
}

/**
 * It takes a picture.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t Q10fGimbalDriver::takePic(void)
{
    const static uint8_t cmd[2] = {0X01, 0XFF};

    return pack(Q10f::GIMBAL_CMD_CAMERA, (uint8_t *)cmd, sizeof(cmd));
}

/**
 * The function sets the video state of the gimbal
 *
 * @param newState The new state of the video.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t Q10fGimbalDriver::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    uint8_t cmd[2] = {0X01, 0XFF};

    if (newState == AMOV_GIMBAL_VIDEO_TAKE)
    {
        cmd[0] = 0X02;
        state.video = AMOV_GIMBAL_VIDEO_TAKE;
    }
    else
    {
        cmd[0] = 0X03;
        state.video = AMOV_GIMBAL_VIDEO_OFF;
    }

    return pack(Q10f::GIMBAL_CMD_CAMERA, (uint8_t *)cmd, sizeof(cmd));
}

uint32_t Q10fGimbalDriver::setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    uint8_t cmd[5] = {0X00, 0X00, 0X00, 0X00, 0XFF};
    if (targetRate == 0.0f)
    {
        cmd[1] = 0XFF;
        switch (zoom)
        {
        case AMOV_GIMBAL_ZOOM_IN:
            cmd[0] = Q10f::GIMBAL_CMD_ZOOM_IN;
            break;
        case AMOV_GIMBAL_ZOOM_OUT:
            cmd[0] = Q10f::GIMBAL_CMD_ZOOM_OUT;
            break;
        case AMOV_GIMBAL_ZOOM_STOP:
            cmd[0] = Q10f::GIMBAL_CMD_ZOOM_STOP;
            break;
        default:
            break;
        }
        return pack(Q10f::GIMBAL_CMD_ZOOM, (uint8_t *)cmd, 2);
    }
    else
    {
        uint16_t count = (targetRate / Q10F_MAX_ZOOM) * Q10F_MAX_ZOOM_COUNT;
        cmd[0] = count & 0XF000 >> 12;
        cmd[1] = count & 0X0F00 >> 8;
        cmd[2] = count & 0X00F0 >> 4;
        cmd[3] = count & 0X000F >> 0;
        return pack(Q10f::GIMBAL_CMD_ZOOM_DIRECT, (uint8_t *)cmd, 5);
    }
}

uint32_t Q10fGimbalDriver::setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    uint8_t cmd[2] = {0X00, 0XFF};
    switch (zoom)
    {
    case AMOV_GIMBAL_ZOOM_IN:
        cmd[0] = Q10f::GIMBAL_CMD_ZOOM_IN;
        break;
    case AMOV_GIMBAL_ZOOM_OUT:
        cmd[0] = Q10f::GIMBAL_CMD_ZOOM_OUT;
        break;
    case AMOV_GIMBAL_ZOOM_STOP:
        cmd[0] = Q10f::GIMBAL_CMD_ZOOM_STOP;
        break;
    default:
        break;
    }
    return pack(Q10f::GIMBAL_CMD_FOCUS, (uint8_t *)cmd, 2);
}
