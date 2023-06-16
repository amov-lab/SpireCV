/*
 * @Description    :
 * @Author         : Aiyangsky
 * @Date           : 2022-08-26 21:42:02
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-03-03 16:12:37
 * @FilePath: \host\gimbal-sdk-multi-platform\src\FIFO\Ring_Fifo.h
 */

#ifndef RING_FIFO_H
#define RING_FIFO_H

#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LOCK()
#define UNLOCK()

    typedef struct
    {
        unsigned char *start;
        unsigned char *in;
        unsigned char *out;
        unsigned char *end;

        unsigned short curr_number;
        unsigned short max_number;
        unsigned short cell_size;
    } RING_FIFO_CB_T;

    void Ring_Fifo_init(RING_FIFO_CB_T *fifo, unsigned short cell_size,
                        unsigned char *buffer, unsigned int buffer_lenght);
    bool Ring_Fifo_in_cell(RING_FIFO_CB_T *fifo, void *data);
    unsigned short Ring_Fifo_in_cells(RING_FIFO_CB_T *fifo, void *data, unsigned short number);
    bool Ring_Fifo_out_cell(RING_FIFO_CB_T *fifo, void *data);
    unsigned short Ring_Fifo_out_cells(RING_FIFO_CB_T *fifo, void *data, unsigned short number);


#ifdef __cplusplus
}
#endif

#endif