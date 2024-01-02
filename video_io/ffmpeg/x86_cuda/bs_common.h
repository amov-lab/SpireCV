#pragma once
#include <string>
#include <vector>
#include <chrono>

// 获取当前系统启动以来的毫秒数
static int64_t getCurTime()
{
    // tv_sec (s) tv_nsec (ns-纳秒)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
}



struct VideoFrame
{
public:
    enum VideoFrameType
    {
        BGR = 0,
        YUV420P,

    };
    // VideoFrame(VideoFrameType type, int width, int height,int size)
    VideoFrame(VideoFrameType type, int width, int height)
    {
        this->type = type;
        this->width = width;
        this->height = height;
        this->size = width*height*3;
        this->data = new uint8_t[this->size];
    }
    ~VideoFrame()
    {
        delete[] this->data;
        this->data = nullptr;
    }

    VideoFrameType type;
    int size;
    int width;
    int height;
    uint8_t *data;
};



