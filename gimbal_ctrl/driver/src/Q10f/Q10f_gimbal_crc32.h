/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2022-10-27 18:10:06
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:28:29
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/Q10f/Q10f_gimbal_crc32.h
 */
#ifndef Q10F_GIMBAL_CRC32_H
#define Q10F_GIMBAL_CRC32_H
namespace Q10f
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

} // namespace name

#endif