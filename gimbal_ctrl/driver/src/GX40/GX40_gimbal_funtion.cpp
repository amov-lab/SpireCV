/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-11-02 17:50:26
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:29:13
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/GX40/GX40_gimbal_funtion.cpp
 */
#include <string.h>
#include "GX40_gimbal_driver.h"

/**
 * The function sets the target position of a gimbal based on the input roll, pitch, and yaw values.
 *
 * @param pos The parameter "pos" is of type "AMOV_GIMBAL_POS_T". It is a structure that
 * contains the roll, pitch, and yaw values of the gimbal position.
 *
 * @return a packed value of type uint32_t.
 */
uint32_t GX40GimbalDriver::setGimabalPos(const AMOV_GIMBAL_POS_T &pos)
{
    carrierStateMutex.lock();
    targetPos[0] = (int16_t)(pos.roll / 0.01f);
    targetPos[1] = (int16_t)(-pos.pitch / 0.01f);
    targetPos[2] = (int16_t)(pos.yaw / 0.01f);
    carrierStateMutex.unlock();
    return pack(GX40::GIMBAL_CMD_MODE_EULER, nullptr, 0);
}

/**
 * The function sets the gimbal speed based on the provided roll, pitch, and yaw values.
 *
 * @param speed The parameter "speed" is of type "AMOV_GIMBAL_POS_T". It is a structure
 * that contains the roll, pitch, and yaw values of the gimbal speed.
 *
 * @return the result of the pack() function, which is of type uint32_t.
 */
uint32_t GX40GimbalDriver::setGimabalSpeed(const AMOV_GIMBAL_POS_T &speed)
{
    carrierStateMutex.lock();
    targetPos[0] = (int16_t)(speed.roll / 0.1f);
    targetPos[1] = (int16_t)(-speed.pitch / 0.1f);
    targetPos[2] = (int16_t)(speed.yaw / 0.1f);
    carrierStateMutex.unlock();

    return pack(GX40::GIMBAL_CMD_MODE_FOLLOW, nullptr, 0);
}

/**
 * The function sets the gimbal's home position to (0, 0, 0) and sends a command to the gimbal to go to
 * the home position.
 *
 * @return the result of the pack() function call with the arguments GX40::GIMBAL_CMD_HOME, nullptr, and
 * 0.
 */
uint32_t GX40GimbalDriver::setGimabalHome(void)
{
    carrierStateMutex.lock();
    targetPos[0] = 0;
    targetPos[1] = 0;
    targetPos[2] = 0;
    carrierStateMutex.unlock();

    pack(GX40::GIMBAL_CMD_MODE_FOLLOW, nullptr, 0);
    return pack(GX40::GIMBAL_CMD_HOME, nullptr, 0);
}

/**
 * The function `takePic` in the `GX40GimbalDriver` class takes a picture using the GX40 gimbal and
 * returns the packed command.
 *
 * @return a uint32_t value.
 */
uint32_t GX40GimbalDriver::takePic(void)
{
    uint8_t temp = 0X01;

    return pack(GX40::GIMBAL_CMD_TAKEPIC, &temp, 1);
}

/**
 * The function `setVideo` toggles the video state of a gimbal driver and returns a packed command.
 *
 * @param newState The parameter `newState` is of type `AMOV_GIMBAL_VIDEO_T`, which is an
 * enumeration representing the state of the video in the gimbal. It can have two possible values:
 *
 * @return the result of the `pack` function, which is a `uint32_t` value.
 */
uint32_t GX40GimbalDriver::setVideo(const AMOV_GIMBAL_VIDEO_T newState)
{
    uint8_t temp = 0X01;

    mState.lock();
    if (state.video == AMOV_GIMBAL_VIDEO_TAKE)
    {
        state.video = AMOV_GIMBAL_VIDEO_OFF;
    }
    else
    {
        state.video = AMOV_GIMBAL_VIDEO_TAKE;
    }
    mState.unlock();

    return pack(GX40::GIMBAL_CMD_TAKEPIC, &temp, 1);
}

/**
 * The function `attitudeCorrection` updates the state of a gimbal driver with position, velocity, and
 * acceleration data.
 *
 * @param pos The "pos" parameter is of type "AMOV_GIMBAL_POS_T" and represents the current
 * position of the gimbal. It likely contains information such as the pitch, yaw, and roll angles of
 * the gimbal.
 * @param seppd The parameter `seppd` stands for "Separate Pointing Device" and represents the velocity
 * of the gimbal in separate axes (e.g., pitch, yaw, roll). It is of type
 * `AMOV_GIMBAL_VELOCITY_T`.
 * @param acc The "acc" parameter is of type "AMOV_GIMBAL_VELOCITY_T" and represents the
 * acceleration of the gimbal.
 * @param extenData The extenData parameter is a pointer to additional data that can be passed to the
 * attitudeCorrection function. It can be used to provide any extra information or context that may be
 * needed for the attitude correction calculation. The specific type and structure of the extenData is
 * not provided in the code snippet,
 *
 * @return the size of the data being passed as arguments. The size is calculated by adding the sizes
 * of the three types: sizeof(AMOV_GIMBAL_POS_T),
 * sizeof(AMOV_GIMBAL_VELOCITY_T), and sizeof(AMOV_GIMBAL_VELOCITY_T).
 */
uint32_t GX40GimbalDriver::attitudeCorrection(const AMOV_GIMBAL_POS_T &pos,
                                              const AMOV_GIMBAL_VELOCITY_T &seppd,
                                              const AMOV_GIMBAL_VELOCITY_T &acc,
                                              void *extenData)
{
    carrierStateMutex.lock();
    carrierPos = pos;
    carrierSpeed = seppd;
    carrierAcc = acc;
    upDataTs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    carrierStateMutex.unlock();
    return sizeof(AMOV_GIMBAL_POS_T) + sizeof(AMOV_GIMBAL_VELOCITY_T) + sizeof(AMOV_GIMBAL_VELOCITY_T);
}

/**
 * The function `extensionFuntions` takes a command as input, converts it to a specific format, and
 * returns a 32-bit unsigned integer.
 *
 * @param cmd The parameter "cmd" is a void pointer, which means it can point to any type of data. In
 * this case, it is being cast to a uint8_t pointer, which means it is expected to point to an array of
 * uint8_t (8-bit unsigned integers).
 *
 * @return a value of type uint32_t.
 */
uint32_t GX40GimbalDriver::extensionFuntions(void *cmd)
{
    uint8_t *temp = (uint8_t *)cmd;
    return pack(temp[0], &temp[2], temp[1]);
}

/**
 * The function `setGimbalZoom` in the `GX40GimbalDriver` class sets the zoom level of a gimbal based on
 * the specified zoom type and target rate.
 *
 * @param zoom The "zoom" parameter is of type AMOV_GIMBAL_ZOOM_T, which is an enumeration
 * type. It represents the zoom action to be performed on the gimbal. The possible values for this
 * parameter are:
 * @param targetRate The targetRate parameter is a float value representing the desired zoom rate for
 * the gimbal. It is used to control the zoom functionality of the gimbal.
 *
 * @return a value of type uint32_t.
 */
uint32_t GX40GimbalDriver::setGimbalZoom(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    uint8_t temp[4];
    uint8_t len = 0;
    temp[1] = 0X01;
    if (targetRate == 0.0f)
    {
        len = 1;
        switch (zoom)
        {
        case AMOV_GIMBAL_ZOOM_IN:
            temp[0] = GX40::GIMBAL_CMD_ZOMM_IN;
            break;
        case AMOV_GIMBAL_ZOOM_OUT:
            temp[0] = GX40::GIMBAL_CMD_ZOOM_OUT;
            break;
        case AMOV_GIMBAL_ZOOM_STOP:
            temp[0] = GX40::GIMBAL_CMD_ZOOM_STOP;
            break;
        }
    }
    else
    {
        len = 3;
        temp[0] = GX40::GIMBAL_CMD_ZOOM;
        int16_t targetTemp = (int16_t)(-targetRate / 0.1f);
        temp[2] = (targetTemp >> 0) & 0XFF;
        temp[3] = (targetTemp >> 8) & 0XFF;
    }

    return pack(temp[0], &temp[1], len);
}

/**
 * The function "setGimbalFocus" sets the focus of a gimbal by specifying the zoom level and target
 * rate.
 *
 * @param zoom The zoom parameter is of type AMOV_GIMBAL_ZOOM_T, which is an enumeration
 * type representing different zoom levels for the gimbal. It is used to specify the desired zoom level
 * for the gimbal focus.
 * @param targetRate The targetRate parameter is a float value representing the desired zoom rate for
 * the gimbal.
 *
 * @return the result of the pack() function, which is of type uint32_t.
 */
uint32_t GX40GimbalDriver::setGimbalFocus(AMOV_GIMBAL_ZOOM_T zoom, float targetRate)
{
    uint8_t temp = 0X01;

    return pack(GX40::GIMBAL_CMD_FOCUE, &temp, 1);
}

/**
 * The function sets the GNSS information in the carrierGNSS struct and returns the size of the struct.
 *
 * @param lng The "lng" parameter represents the longitude value of the GNSS (Global Navigation
 * Satellite System) information.
 * @param lat The "lat" parameter represents the latitude value of the GNSS (Global Navigation
 * Satellite System) information.
 * @param alt The "alt" parameter represents the altitude value in meters.
 * @param nState The parameter "nState" represents the state of the GNSS (Global Navigation Satellite
 * System) information. It is of type uint32_t, which means it is an unsigned 32-bit integer. The
 * specific values and their meanings for the "nState" parameter are not provided in the code snippet
 * @param relAlt Relative altitude of the carrier (in meters)
 *
 * @return the size of the structure GX40::GIMBAL_SECONDARY_MASTER_FRAME_T.
 */
uint32_t GX40GimbalDriver::setGNSSInfo(float lng, float lat, float alt, uint32_t nState, float relAlt)
{
    carrierStateMutex.lock();
    carrierGNSS.head = 0X01;

    carrierGNSS.lng = lng / 1E-7;
    carrierGNSS.lat = lat / 1E-7;
    carrierGNSS.alt = alt / 1E-3;

    carrierGNSS.relAlt = relAlt / 1E-3;
    carrierGNSS.nState = nState;

    carrierStateMutex.unlock();
    return sizeof(GX40::GIMBAL_SECONDARY_MASTER_FRAME_T);
}
