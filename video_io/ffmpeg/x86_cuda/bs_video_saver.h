#pragma once

#include <iostream>
#include <thread>         
#include <chrono> 


#include <fmt/core.h>
// #include <glog/logging.h>

#include <queue>
#include <mutex>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
// #include <libavutil/error.h>
#include <libswresample/swresample.h>
}

#include <opencv2/core/core.hpp>

#include "bs_common.h"


class BsVideoSaver
{
public:
    BsVideoSaver();
    ~BsVideoSaver();

    // 用于初始化视频推流，仅调用一次
    bool setup(std::string name, int width, int height, int fps, std::string encoder, int bitrate);
    // 推流一帧图像，在循环中被调用
    void write(cv::Mat& image);


    // 连接流媒体服务器
    bool init(std::string name, int width, int height, int fps, std::string encoder, int bitrate);
    void start();
    void stop();

    // 编码视频帧并推流
    static void encodeVideoAndSaveThread(void* arg); 

    bool videoFrameQisEmpty();

    int writePkt(AVPacket *pkt);


    // 上下文
    AVFormatContext *mFmtCtx = nullptr;
    // 视频帧
    AVCodecContext *mVideoCodecCtx = NULL;
    AVStream *mVideoStream = NULL;


    int mVideoIndex = -1;


private:

    // 从mRGB_VideoFrameQ里面获取RGBframe
    bool getVideoFrame(VideoFrame *&frame, int &frameQSize); 


    // bgr24转yuv420p
    unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char max_val);
    bool bgr24ToYuv420p(unsigned char *bgrBuf, int w, int h, unsigned char *yuvBuf);

    int width = -1;
    int height = -1;


    bool push_running = false;
    bool nd_push_frame = false;

    // 视频帧
    std::queue<VideoFrame *> mRGB_VideoFrameQ;
    std::mutex mRGB_VideoFrameQ_mtx;


    // 推流锁
    std::mutex mWritePkt_mtx;
    std::thread* mThread = nullptr;


};