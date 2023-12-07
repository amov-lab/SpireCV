/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:30:27
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/AT10/AT10_gimbal_crc32.h
 */
#ifndef AT10_GIMBAL_CRC32_H
#define AT10_GIMBAL_CRC32_H

namespace AT10
{
    static inline unsigned char CheckSum(unsigned char *pData, unsigned short Lenght)
    {
        unsigned short temp = 0;
        unsigned short i = 0;
        for (i = 0; i < Lenght; i++)
        {
            temp += pData[i];
        }
        return temp & 0XFF;
    }

    static inline unsigned char checkXOR(unsigned char *pData, unsigned char Lenght)
    {
        unsigned char temp = Lenght;
        unsigned char i;
        for (i = 1; i < Lenght - 1; i++)
        {
            temp ^= pData[i];
        }
        return temp;
    }

} // namespace name

#endif