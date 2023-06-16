/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-03-02 11:16:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-04-18 10:13:08
 * @FilePath: /gimbal-sdk-multi-platform/src/G2/g2_gimbal_iap_funtion.cpp
 */

#ifdef  AMOV_HOST

#include "g2_gimbal_driver.h"
#include "g2_gimbal_crc.h"
#include "string.h"

#include <chrono>

#define MAX_WAIT_TIME_MS 2000

/**
 * It gets the software information from the gimbal.
 *
 * @param info the string to store the information
 *
 * @return a boolean value.
 */
bool g2GimbalDriver::iapGetSoftInfo(std::string &info)
{
    uint8_t temp = 0;
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    pack(G2::IAP_COMMAND_SOFT_INFO, &temp, 1);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_SOFT_INFO &&
                ack.target == self &&
                ack.source == remote)
            {
                info = (char *)ack.data;
                std::cout << info << std::endl;
                ret = true;
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * It gets the hardware information of the gimbal.
 *
 * @param info the string to store the hardware information
 *
 * @return a boolean value.
 */
bool g2GimbalDriver::iapGetHardInfo(std::string &info)
{
    uint8_t temp = 0;
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    pack(G2::IAP_COMMAND_HARDWARE_INFO, &temp, 1);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_HARDWARE_INFO &&
                ack.target == self &&
                ack.source == remote)
            {
                info = (char *)ack.data;
                std::cout << info << std::endl;
                ret = true;
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * It sends a command to the gimbal to jump to the bootloader, and then waits for a response from the
 * bootloader
 *
 * @param state the state of the gimbal, 0: normal, 1: iap
 *
 * @return The return value is a boolean.
 */
bool g2GimbalDriver::iapJump(G2::GIMBAL_IAP_STATE_T &state)
{
    uint8_t temp = 0;
    bool ret = true;
    G2::GIMBAL_FRAME_T ack;

    pack(G2::IAP_COMMAND_JUMP, &temp, 1);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // It fails if the specified message is received.
    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_JUMP &&
                ack.target == self &&
                ack.source == remote)
            {
                state = (G2::GIMBAL_IAP_STATE_T)ack.data[1];
                ret = false;
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * The function sends a command to the gimbal to erase the flash memory
 *
 * @param state The state of the IAP process.
 *
 * @return The return value is a boolean.
 */
bool g2GimbalDriver::iapFlashErase(G2::GIMBAL_IAP_STATE_T &state)
{
    uint8_t temp = 0;
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    pack(G2::IAP_COMMAND_FLASH_ERASE, &temp, 1);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_FLASH_ERASE &&
                ack.target == self &&
                ack.source == remote)
            {
                state = (G2::GIMBAL_IAP_STATE_T)ack.data[1];
                ret = true;
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * It sends a block of data to the gimbal, and waits for an acknowledgement
 *
 * @param startAddr the start address of the block to be sent
 * @param crc32 The CRC32 of the data to be sent.
 *
 * @return a boolean value.
 */
bool g2GimbalDriver::iapSendBlockInfo(uint32_t &startAddr, uint32_t &crc32)
{
    union
    {
        uint32_t f32;
        uint8_t f8[4];
    } temp;
    uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    temp.f32 = startAddr;
    memcpy(buf, temp.f8, sizeof(uint32_t));
    temp.f32 = crc32;
    memcpy(buf, temp.f8, sizeof(uint32_t));

    pack(G2::IAP_COMMAND_BOLCK_INFO, buf, 8);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_BOLCK_INFO &&
                ack.target == self &&
                ack.source == remote)
            {
                ret = true;
                for (uint8_t i = 0; i < 8; i++)
                {
                    if (buf[i] != ack.data[i])
                    {
                        ret = false;
                    }
                }
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * It sends a block of data to the gimbal, and waits for an acknowledgement
 *
 * @param offset the offset of the data block in the file
 * @param data pointer to the data to be sent
 *
 * @return The return value is a boolean.
 */
bool g2GimbalDriver::iapSendBlockData(uint8_t offset, uint8_t *data)
{
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    pack(G2::IAP_COMMAND_BLOCK_START + offset, data, 64);

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_BLOCK_START + offset &&
                ack.target == self &&
                ack.source == remote)
            {
                ret = true;
                for (uint8_t i = 0; i < 64; i++)
                {
                    if (data[i] != ack.data[i])
                    {
                        ret = false;
                    }
                }
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

/**
 * The function sends a block of data to the gimbal, and waits for an acknowledgement
 * 
 * @param crc32 The CRC32 of the data block
 * @param state The state of the IAP process.
 * 
 * @return The return value is a boolean.
 */
bool g2GimbalDriver::iapFlashWrite(uint32_t &crc32, G2::GIMBAL_IAP_STATE_T &state)
{
    bool ret = false;
    G2::GIMBAL_FRAME_T ack;

    union
    {
        uint32_t f32;
        uint8_t f8[4];
    } temp;

    temp.f32 = crc32;

    pack(G2::IAP_COMMAND_BLOCK_WRITE, temp.f8, sizeof(uint32_t));

    std::chrono::milliseconds startMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (1)
    {
        if (getRxPack(&ack))
        {
            if (ack.command == G2::IAP_COMMAND_BLOCK_WRITE &&
                ack.target == self &&
                ack.source == remote)
            {
                state = (G2::GIMBAL_IAP_STATE_T)ack.data[4];
                ret = true;
                for (uint8_t i = 0; i < 4; i++)
                {
                    if (temp.f8[i] != ack.data[i])
                    {
                        ret = false;
                    }
                }
                break;
            }
        }

        std::chrono::milliseconds nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        if ((nowMs - startMs) > std::chrono::milliseconds(MAX_WAIT_TIME_MS))
        {
            break;
        }
    }
    return ret;
}

#endif

