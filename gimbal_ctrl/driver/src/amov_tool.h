/*
 * @Description:
 * @Author: L LC @amov
 * @Date: 2023-07-31 18:30:33
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:26:05
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_tool.h
 */

#ifndef __AMOVGIMABL_TOOL_H
#define __AMOVGIMABL_TOOL_H
namespace amovGimbalTools
{
    static inline unsigned short conversionBigLittle(unsigned short value)
    {
        unsigned short temp = 0;
        temp |= ((value >> 8) & 0X00FF);
        temp |= ((value << 8) & 0XFF00);
        return temp;
    }

    static inline unsigned int conversionBigLittle(unsigned int value)
    {
        unsigned int temp = 0;
        temp |= ((value << 24) & 0XFF000000);
        temp |= ((value << 8) & 0X00FF0000);
        temp |= ((value >> 8) & 0X0000FF00);
        temp |= ((value << 24) & 0X000000FF);
        return temp;
    }
}
#endif
