/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-02 10:00:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-11-27 16:27:18
 * @FilePath: /gimbal-sdk-multi-platform/src/AT10/AT10_gimbal_funtion.cpp
 */
#include "AT10_gimbal_driver.h"
#include "AT10_gimbal_crc32.h"
#include "string.h"
#include "math.h"
/**
 * It sets the gimbal position.
 *
 * @param pos the position of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t AT10GimbalDriver::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    int16_t yaw, pitch;
    AT10::GIMBAL_CMD_A1_MSG_T temp;
    yaw = (int16_t)(pos.yaw / (360.0f / 65536.0f));
    printf("\r\n %04X\r\n", yaw);
    yaw = amovGimbalTools::conversionBigLittle((uint16_t)yaw);
    pitch = (int16_t)(pos.pitch / (360.0f / 65536.0f));
    pitch = amovGimbalTools::conversionBigLittle((uint16_t)pitch);
    temp.cmd = 0x0B;
    temp.param[0] = yaw;
    temp.param[1] = pitch;
    temp.param[2] = 0;
    temp.param[3] = 0;
    return pack(AT10::GIMBAL_CMD_STD_MOTOR, (uint8_t *)&temp, sizeof(AT10::GIMBAL_CMD_A1_MSG_T));
}

/**
 * It takes a struct of type AMOV_GIMBAL_POS_T and converts it to a struct of type
 * G1::GIMBAL_SET_POS_MSG_T
 *
 * @param speed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t AT10GimbalDriver::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    int16_t speedYaw, speedPitch;
    AT10::GIMBAL_CMD_A1_MSG_T temp;
    speedYaw = (int16_t)(speed.yaw * 100);
    printf("\r\n %04X\r\n", speedYaw);
    speedYaw = amovGimbalTools::conversionBigLittle((uint16_t)speedYaw);
    speedPitch = (int16_t)(speed.pitch * 100);
    speedPitch = amovGimbalTools::conversionBigLittle((uint16_t)speedPitch);
    temp.cmd = 0x01;
    temp.param[0] = speedYaw;
    temp.param[1] = speedPitch;
    temp.param[2] = 0;
    temp.param[3] = 0;
    return pack(AT10::GIMBAL_CMD_STD_MOTOR, (uint8_t *)&temp, sizeof(AT10::GIMBAL_CMD_A1_MSG_T));
}

/**
 * This function sets the gimbal's follow speed
 *
 * @param followSpeed the speed of the gimbal
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t AT10GimbalDriver::setGimabalFollowSpeed(const AMOV_GIMBAL_POS_T &followSpeed)
{
    state.maxFollow.pitch = fabs(followSpeed.pitch * 100);
    state.maxFollow.yaw = fabs(followSpeed.yaw * 100);
    state.maxFollow.roll = fabs(followSpeed.roll * 100);
    return 0;
}

/**
 * This function sets the gimbal to its home position
 *
 * @return The return value is the number of bytes written to the buffer.
 */
uint32_t AT10GimbalDriver::setGimabalHome(void)
{
    AT10::GIMBAL_CMD_A1_MSG_T temp;
    temp.cmd = 0x04;
    temp.param[0] = 0;
    temp.param[1] = 0;
    temp.param[2] = 0;
    temp.param[3] = 0;
    return pack(AT10::GIMBAL_CMD_STD_MOTOR, (uint8_t *)&temp, sizeof(AT10::GIMBAL_CMD_A1_MSG_T));
}

/**
 * It takes a picture.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t AT10GimbalDriver::takePic(void)
{
    uint16_t temp = 0x13 << 3;
    uint16_t data = amovGimbalTools::conversionBigLittle(temp);

    return pack(AT10::GIMBAL_CMD_STD_CAMERA, (uint8_t *)&data, sizeof(uint16_t));
}

/**
 * The function sets the video state of the gimbal
 *
 * @param newState The new state of the video.
 *
 * @return The return value is the number of bytes written to the serial port.
 */
uint32_t AT10GimbalDriver::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    uint16_t temp;
    if (newState == AMOV_GIMBAL_VIDEO_T::AMOV_GIMBAL_VIDEO_TAKE)
    {
        temp = 0x14 << 3;
    }
    else
    {
        temp = 0x15 << 3;
    }

    uint16_t data = amovGimbalTools::conversionBigLittle(temp);

    return pack(AT10::GIMBAL_CMD_STD_CAMERA, (uint8_t *)&data, sizeof(uint16_t));
}

uint32_t AT10GimbalDriver::setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    if (targetRate == 0.0f)
    {
        uint16_t temp = 0;
        switch (zoom)
        {
        case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_IN:
            temp = 0X08 << 3;
            break;
        case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_OUT:
            temp = 0X09 << 3;
            break;
        case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_STOP:
            temp = 0X01 << 3;
            break;
        default:
            break;
        }
        uint16_t data = amovGimbalTools::conversionBigLittle(temp);

        return pack(AT10::GIMBAL_CMD_STD_CAMERA, (uint8_t *)&data, sizeof(uint16_t));
    }
    else
    {
        AT10::GIMBAL_CMD_C2_MSG_T temp;
        temp.cmd = 0x53;
        temp.param = targetRate * 10;
        temp.param = amovGimbalTools::conversionBigLittle(temp.param);
        return pack(AT10::GIMBAL_CMD_STD_CAMERA2, (uint8_t *)&temp, sizeof(AT10::GIMBAL_CMD_C2_MSG_T));
    }
}

uint32_t AT10GimbalDriver::setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    uint16_t temp = 0;
    switch (zoom)
    {
    case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_IN:
        temp = 0X0B << 3;
        break;
    case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_OUT:
        temp = 0X0A << 3;
        break;
    case AMOV_GIMBAL_ZOOM_T::AMOV_GIMBAL_ZOOM_STOP:
        temp = 0X01 << 3;
        break;
    default:
        break;
    }
    uint16_t data = amovGimbalTools::conversionBigLittle(temp);

    return pack(AT10::GIMBAL_CMD_STD_CAMERA, (uint8_t *)&data, sizeof(uint16_t));
}

//
/**
 * The function `extensionFuntions` in the `AT10GimbalDriver` class takes a command as input, casts it
 * to a specific type, and then packs the command and its parameters into a byte array.
 *
 * @param cmd The "cmd" parameter is a void pointer, which means it can point to any type of data. In
 * this case, it is being cast to a pointer of type AT10::AT10_EXT_CMD_T using the reinterpret_cast
 * operator.
 *
 * @return the result of the `pack` function, which is of type `uint32_t`.
 */
uint32_t AT10GimbalDriver::extensionFuntions(void *cmd)
{
    AT10::AT10_EXT_CMD_T *tempCMD;
    tempCMD = reinterpret_cast<AT10::AT10_EXT_CMD_T *>(cmd);

    return pack(tempCMD->cmd, (uint8_t *)tempCMD->param, tempCMD->paramLen);
}

// AT10_EXT_CMD_T infraredOpen ;
// infraredOpen.cmd = AT10::GIMBAL_CMD_STD_CAMERA;
// infraredOpen.param[0] = 0X02;
// infraredOpen.param[1] = 0;
// infraredOpen.paramLen = 2;