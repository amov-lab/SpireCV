/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-10-20 16:08:17
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-06 10:27:28
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/GX40/GX40_gimbal_driver.cpp
 */
#include <string.h>
#include "GX40_gimbal_driver.h"
#include "GX40_gimbal_crc16.h"
#include <math.h>

/**
 * The above function is a constructor for the GX40GimbalDriver class in C++, which initializes member
 * variables and sets the parser state to idle.
 *
 * @param _IO _IO is a pointer to an object of type amovGimbal::IOStreamBase. It is used to communicate
 * with the gimbal device.
 */
GX40GimbalDriver::GX40GimbalDriver(amovGimbal::IOStreamBase *_IO) : amovGimbal::amovGimbalBase(_IO)
{
    rxQueue = new fifoRing(sizeof(GX40::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);
    txQueue = new fifoRing(sizeof(GX40::GIMBAL_FRAME_T), MAX_QUEUE_SIZE);

    targetPos[0] = 0;
    targetPos[1] = 0;
    targetPos[2] = 0;

    parserState = GX40::GIMBAL_FRAME_PARSER_STATE_IDLE;
}

/**
 * The function `nopSend` continuously sends a "no operation" command to a GX40 gimbal driver.
 */
void GX40GimbalDriver::nopSend(void)
{
    while (1)
    {
        // 50Hz
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pack(GX40::GIMBAL_CMD_NOP, nullptr, 0);
    }
}

/**
 * The function `parserStart` initializes the gimbal driver by setting the callback function, creating
 * two threads for the main loop and sending NOP commands, and detaching the threads.
 *
 * @param callback The parameter "callback" is of type "amovGimbal::pStateInvoke", which is a function
 * pointer type. It is used to specify a callback function that will be invoked when the gimbal state
 * is updated.
 */
void GX40GimbalDriver::parserStart(pAmovGimbalStateInvoke callback, void *caller)
{
    this->updateGimbalStateCallback = callback;
    this->updataCaller = caller;

    std::thread mainLoop(&GX40GimbalDriver::mainLoop, this);
    std::thread sendNop(&GX40GimbalDriver::nopSend, this);

    this->stackThreadHanle = mainLoop.native_handle();
    this->nopSendThreadHandle = sendNop.native_handle();

    mainLoop.detach();
    sendNop.detach();
}

/**
 * The function `pack` in the `GX40GimbalDriver` class is responsible for packing data into a frame for
 * transmission.
 *
 * @param uint32_t The parameter `cmd` is an unsigned 32-bit integer representing the command.
 * @param pPayload The `pPayload` parameter is a pointer to the payload data that needs to be packed.
 * It is of type `uint8_t*`, which means it is a pointer to an array of unsigned 8-bit integers. The
 * payload data is stored in this array.
 * @param payloadSize The parameter `payloadSize` represents the size of the payload data in bytes. It
 * is used to determine the size of the payload data that needs to be packed into the `temp` structure.
 *
 * @return a uint32_t value, which is stored in the variable "ret".
 */
uint32_t GX40GimbalDriver::pack(IN uint32_t cmd, uint8_t *pPayload, uint8_t payloadSize)
{
    uint32_t ret = 0;
    GX40::GIMBAL_FRAME_T temp;
    memset(&temp, 0, sizeof(GX40::GIMBAL_FRAME_T));
    GX40::GIMBAL_PRIMARY_MASTER_FRAME_T *primary = (GX40::GIMBAL_PRIMARY_MASTER_FRAME_T *)temp.primaryData;
    carrierStateMutex.lock();

    primary->state = 0X00;
    // 姿态数据&指令数据填充
    primary->roll = targetPos[0];
    primary->pitch = targetPos[1];
    primary->yaw = targetPos[2];

    primary->state |= (0X01 << 2);

    temp.otherData[0] = cmd;
    memcpy(temp.otherData + 1, pPayload, payloadSize);

    // 固定字节填充
    temp.head.u16 = XF_SEND_HEAD;
    temp.version = 0X01;
    primary->secondaryFlag = 0X01;
    temp.lenght.u16 = 69 + payloadSize + 1 + 2;

    // 惯导数据填充
    std::chrono::milliseconds nowTs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    // over 1s GNSS has losed
    if ((nowTs.count() - upDataTs.count()) < std::chrono::milliseconds(1500).count())
    {
        primary->selfRoll = (int16_t)(-(carrierPos.roll / 0.01f));
        primary->selfPitch = (int16_t)(-(carrierPos.pitch / 0.01f));
        primary->selfYaw = (int16_t)(carrierPos.yaw / 0.01f);
        primary->accE = (int16_t)(carrierAcc.y / 0.01f);
        primary->accN = (int16_t)(carrierAcc.x / 0.01f);
        primary->accUp = (int16_t)(carrierAcc.z / 0.01f);
        primary->speedE = (int16_t)(carrierSpeed.y / 0.01f);
        primary->speedN = (int16_t)(carrierSpeed.x / 0.01f);
        primary->speedUp = (int16_t)(carrierSpeed.z / 0.01f);

        carrierGNSS.GPSweeks = ((nowTs.count() / 1000) - 315964800) / 604800;
        carrierGNSS.GPSms = nowTs.count() - (carrierGNSS.GPSweeks * 604800000);

        memcpy(temp.secondaryData, &carrierGNSS, sizeof(GX40::GIMBAL_SECONDARY_MASTER_FRAME_T));

        primary->state |= (0X01 << 0);
    }
    else
    {
        primary->state &= (~(0X01 << 0));
    }

    carrierStateMutex.unlock();

    // 校验
    *(uint16_t *)(&temp.otherData[payloadSize + 1]) = GX40::CalculateCrc16((uint8_t *)&temp, 69 + 1 + payloadSize);

    // 添加至发送队列
    if (txQueue->inCell(&temp))
    {
        ret = temp.lenght.u16;
    }
    return ret;
}

/**
 * The function `convert` takes a buffer and extracts data from it to update the state of a gimbal
 * driver.
 *
 * @param buf The `buf` parameter is a void pointer that points to a buffer containing data that needs
 * to be converted.
 */
void GX40GimbalDriver::convert(void *buf)
{
    GX40::GIMBAL_FRAME_T *temp;
    GX40::GIMBAL_PRIMARY_SLAVE_FRAME_T *primary;
    GX40::GIMBAL_SECONDARY_SLAVE_FRAME_T *secondary;
    temp = reinterpret_cast<GX40::GIMBAL_FRAME_T *>(buf);
    primary = (GX40::GIMBAL_PRIMARY_SLAVE_FRAME_T *)temp->primaryData;
    secondary = (GX40::GIMBAL_SECONDARY_SLAVE_FRAME_T *)temp->secondaryData;

    mState.lock();
    this->state.workMode = (AMOV_GIMBAL_SERVO_MODE_T)primary->workMode;
    this->state.cameraFlag = (AMOV_GIMBAL_CAMERA_FLAG_T)primary->state;
    // 应该需要再解算一下，才能出具体的框架角度
    this->state.rel.yaw = -(primary->motorYaw * XF_ANGLE_DPI);
    this->state.rel.yaw = this->state.rel.yaw < -180.0f ? this->state.rel.yaw + 360.0f : this->state.rel.yaw;
    this->state.rel.pitch = -(primary->motorPitch * XF_ANGLE_DPI);
    this->state.rel.roll = -(primary->motorRoll * XF_ANGLE_DPI);

    this->state.abs.yaw = -(primary->yaw * XF_ANGLE_DPI);
    this->state.abs.yaw = this->state.abs.yaw < -180.0f ? this->state.abs.yaw + 360.0f : this->state.abs.yaw;

    this->state.abs.pitch = -(primary->pitch * XF_ANGLE_DPI);
    this->state.abs.roll = -(primary->roll * XF_ANGLE_DPI);

    this->state.relSpeed.yaw = -(primary->speedYaw * XF_ANGLE_DPI);
    this->state.relSpeed.pitch = -(primary->speedPitch * XF_ANGLE_DPI);
    this->state.relSpeed.roll = -(primary->speedRoll * XF_ANGLE_DPI);

    // 近似值 不准
    this->state.fov.x = secondary->camera1Zoom * 0.1f;
    this->state.fov.x = 60.2f / this->state.fov.x;
    this->state.fov.y = secondary->camera1Zoom * 0.1f;
    this->state.fov.y = 36.1f / this->state.fov.y;

    updateGimbalStateCallback(state.rel.roll, state.rel.pitch, state.rel.yaw,
                              state.abs.roll, state.abs.pitch, state.abs.yaw,
                              state.fov.x, state.fov.y, updataCaller);

    mState.unlock();
}

/**
 * The function calculates the total length of a data packet by adding the length of the payload to the
 * size of a uint16_t.
 *
 * @param pack The parameter "pack" is a void pointer, which means it can point to any type of data. In
 * this case, it is expected to point to a structure of type "GX40::GIMBAL_FRAME_T".
 *
 * @return the sum of the length of the gimbal frame and the size of a uint16_t.
 */
uint32_t GX40GimbalDriver::calPackLen(void *pack)
{
    return ((GX40::GIMBAL_FRAME_T *)pack)->lenght.u16;
}
/**
 * The function `parser` is used to parse incoming data frames in a specific format and returns a
 * boolean value indicating whether the parsing was successful or not.
 *
 * @param uint8_t The parameter `byte` is of type `uint8_t`, which is an unsigned 8-bit integer. It is
 * used to store a single byte of data that is being parsed by the `GX40GimbalDriver::parser` function.
 *
 * @return a boolean value, either true or false.
 */
bool GX40GimbalDriver::parser(IN uint8_t byte)
{
    bool state = false;
    static uint8_t payloadLenght = 0;
    static uint8_t *pRx = nullptr;

    switch (parserState)
    {
    case GX40::GIMBAL_FRAME_PARSER_STATE_IDLE:
        if (byte == ((XF_RCV_HEAD >> 8) & 0XFF))
        {
            rx.head.u8[0] = byte;
            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_HEAD;
        }
        break;

    case GX40::GIMBAL_FRAME_PARSER_STATE_HEAD:
        if (byte == ((XF_RCV_HEAD >> 0) & 0XFF))
        {
            rx.head.u8[1] = byte;
            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_LEN1;
        }
        else
        {
            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_IDLE;
            rx.head.u16 = 0;
        }
        break;

    case GX40::GIMBAL_FRAME_PARSER_STATE_LEN1:
        rx.lenght.u8[0] = byte;
        parserState = GX40::GIMBAL_FRAME_PARSER_STATE_LEN2;
        break;

    case GX40::GIMBAL_FRAME_PARSER_STATE_LEN2:
        rx.lenght.u8[1] = byte;
        parserState = GX40::GIMBAL_FRAME_PARSER_STATE_VERSION;
        break;

    case GX40::GIMBAL_FRAME_PARSER_STATE_VERSION:
        if (byte == XF_VERSION)
        {
            rx.version = byte;
            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_PAYLOAD;
            pRx = rx.primaryData;
            payloadLenght = rx.lenght.u16 - 5;
        }
        else
        {
            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_IDLE;
            rx.head.u16 = 0;
            rx.lenght.u16 = 0;
        }
        break;

    case GX40::GIMBAL_FRAME_PARSER_STATE_PAYLOAD:
        *pRx = byte;
        payloadLenght--;
        pRx++;
        if (payloadLenght <= 0)
        {
            if (*(uint16_t *)(pRx - sizeof(uint16_t)) == GX40::CalculateCrc16((uint8_t *)&rx, rx.lenght.u16 - 2))
            {
                state = true;
                rxQueue->inCell(&rx);
            }
            else
            {
                memset(&rx, 0, sizeof(GX40::GIMBAL_FRAME_T));
            }

            parserState = GX40::GIMBAL_FRAME_PARSER_STATE_IDLE;
            pRx = nullptr;
            payloadLenght = 0;
        }
        break;

    default:
        parserState = GX40::GIMBAL_FRAME_PARSER_STATE_IDLE;
        pRx = nullptr;
        payloadLenght = 0;
        break;
    }

    return state;
}