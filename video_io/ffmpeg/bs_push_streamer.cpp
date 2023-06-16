#include "bs_push_streamer.h"

/*
amov_rtsp
2914e3c44737811096c5e1797fe5373d12fcdd39
*/


// char av_error[AV_ERROR_MAX_STRING_SIZE] = { 0 };
// #define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)

BsPushStreamer::BsPushStreamer()
{
   
}

BsPushStreamer::~BsPushStreamer()
{
    mThread->join();
    mThread = nullptr;
}


bool BsPushStreamer::setup(std::string name, int width, int height, int fps, std::string encoder, int bitrate = 4)
{
    if (!connect(name, width, height, fps, encoder, bitrate))
    {
        std::cout << "BsPushStreamer::setup error\n";
        return false;
    }

    mVideoFrame = new VideoFrame(VideoFrame::BGR, width, height);
    // std::cout << "BsStreamer::setup Success!\n";
    start();
    return true;
}

void BsPushStreamer::start()
{
    mThread = new std::thread(BsPushStreamer::encodeVideoAndWriteStreamThread, this);
    mThread->native_handle();
    push_running = true;
}


bool BsPushStreamer::connect(std::string name, int width, int height, int fps, std::string encoder, int bitrate)
{
    // 初始化上下文
    if (avformat_alloc_output_context2(&mFmtCtx, NULL, "rtsp", name.c_str()) < 0)
    {
        std::cout << "avformat_alloc_output_context2 error\n";
        return false;
    }

    // 初始化视频编码器
    // AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    // AVCodec *videoCodec = avcodec_find_encoder_by_name("h264_nvenc");
    
    AVCodec *videoCodec = avcodec_find_encoder_by_name(encoder.c_str());
    if (!videoCodec)
    {
        std::cout << fmt::format("Using encoder:[{}] error!\n", encoder);
        videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);

        if (!videoCodec)
        {
            std::cout << "avcodec_alloc_context3 error";
            return false;
        }
        std::cout << "Using default H264 encoder!\n";
    }

    mVideoCodecCtx = avcodec_alloc_context3(videoCodec);
    if (!mVideoCodecCtx)
    {
        std::cout << "avcodec_alloc_context3 error";
        return false;
    }

    // 压缩视频bit位大小 300kB
    int bit_rate = bitrate * 1024 * 1024 * 8;

    // CBR：Constant BitRate - 固定比特率
    mVideoCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    mVideoCodecCtx->bit_rate = bit_rate;
    mVideoCodecCtx->rc_min_rate = bit_rate;
    mVideoCodecCtx->rc_max_rate = bit_rate;
    mVideoCodecCtx->bit_rate_tolerance = bit_rate;

    mVideoCodecCtx->codec_id = videoCodec->id;
    // 不支持AV_PIX_FMT_BGR24直接进行编码
    mVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    mVideoCodecCtx->width = width;
    mVideoCodecCtx->height = height;
    mVideoCodecCtx->time_base = {1, fps};
    mVideoCodecCtx->framerate = {fps, 1};
    mVideoCodecCtx->gop_size = 12;
    mVideoCodecCtx->max_b_frames = 0;
    mVideoCodecCtx->thread_count = 1;

    // 手动设置PPS
    // unsigned char sps_pps[] = {
    //     0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x2a, 0x96, 0x35, 0x40, 0xf0, 0x04,
    //     0x4f, 0xcb, 0x37, 0x01, 0x01, 0x01, 0x40, 0x00, 0x01, 0xc2, 0x00, 0x00, 0x57,
    //     0xe4, 0x01, 0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80, 0x00
    // };

    AVDictionary *video_codec_options = NULL;
    av_dict_set(&video_codec_options, "profile", "main", 0);
    // av_dict_set(&video_codec_options, "preset", "superfast", 0);
    av_dict_set(&video_codec_options, "tune", "fastdecode", 0);

    if (avcodec_open2(mVideoCodecCtx, videoCodec, &video_codec_options) < 0)
    {
        std::cout << "avcodec_open2 error\n";
        return false;
    }

    mVideoStream = avformat_new_stream(mFmtCtx, videoCodec);
    if (!mVideoStream)
    {
        std::cout << "avformat_new_stream error\n";
        return false;
    }
    mVideoStream->id = mFmtCtx->nb_streams - 1;
    // stream的time_base参数非常重要，它表示将现实中的一秒钟分为多少个时间基, 在下面调用avformat_write_header时自动完成
    avcodec_parameters_from_context(mVideoStream->codecpar, mVideoCodecCtx);
    mVideoIndex = mVideoStream->id;


    // open output url
    av_dump_format(mFmtCtx, 0, name.c_str(), 1);
    if (!(mFmtCtx->oformat->flags & AVFMT_NOFILE))
    {
        int ret = avio_open(&mFmtCtx->pb, name.c_str(), AVIO_FLAG_WRITE);
        if ( ret < 0)
        {
            std::cout << fmt::format("avio_open error url: {}\n", name.c_str());
            // std::cout << fmt::format("ret = {} : {}\n", ret, av_err2str(ret));
            std::cout << fmt::format("ret = {}\n", ret);
            return false;
        }
    }

    AVDictionary *fmt_options = NULL;
    av_dict_set(&fmt_options, "bufsize", "1024", 0);
    av_dict_set(&fmt_options, "rw_timeout", "30000000", 0); // 设置rtmp/http-flv连接超时（单位 us）
    av_dict_set(&fmt_options, "stimeout", "30000000", 0);   // 设置rtsp连接超时（单位 us）
    av_dict_set(&fmt_options, "rtsp_transport", "tcp", 0);

    mFmtCtx->video_codec_id = mFmtCtx->oformat->video_codec;
    mFmtCtx->audio_codec_id = mFmtCtx->oformat->audio_codec;

    // 调用该函数会将所有stream的time_base，自动设置一个值，通常是1/90000或1/1000，这表示一秒钟表示的时间基长度
    if (avformat_write_header(mFmtCtx, &fmt_options) < 0)
    {
        std::cout << "avformat_write_header error\n";
        return false;
    }

    return true;
}

void BsPushStreamer::encodeVideoAndWriteStreamThread(void* arg)
{
    // PushExecutor *executor = (PushExecutor *)arg;
    BsPushStreamer *mBsPushStreamer = (BsPushStreamer *)arg;
    int width = mBsPushStreamer->mVideoFrame->width;
    int height = mBsPushStreamer->mVideoFrame->height;

    // 未编码的视频帧（bgr格式）
    // VideoFrame *videoFrame = NULL;
    // 未编码视频帧队列当前长度
    // int videoFrameQSize = 0;

    AVFrame *frame_yuv420p = av_frame_alloc();
    frame_yuv420p->format = mBsPushStreamer->mVideoCodecCtx->pix_fmt;
    frame_yuv420p->width = width;
    frame_yuv420p->height = height;

    int frame_yuv420p_buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
    uint8_t *frame_yuv420p_buff = (uint8_t *)av_malloc(frame_yuv420p_buff_size);
    av_image_fill_arrays(
        frame_yuv420p->data, frame_yuv420p->linesize,
        frame_yuv420p_buff,
        AV_PIX_FMT_YUV420P,
        width, height, 1);

    // 编码后的视频帧
    AVPacket *pkt = av_packet_alloc();
    int64_t encodeSuccessCount = 0;
    int64_t frameCount = 0;

    int64_t t1 = 0;
    int64_t t2 = 0;
    int ret = -1;

    auto cnt_time = std::chrono::system_clock::now().time_since_epoch();
    auto last_update_time = std::chrono::system_clock::now().time_since_epoch();
 
    while (mBsPushStreamer->push_running)
    {
        // if (mBsPushStreamer->getVideoFrame(videoFrame, videoFrameQSize))
        if (mBsPushStreamer->nd_push_frame)
        {
            mBsPushStreamer->nd_push_frame = false;
            // frame_bgr 转  frame_yuv420p
            mBsPushStreamer->bgr24ToYuv420p(mBsPushStreamer->mVideoFrame->data, width, height, frame_yuv420p_buff);

            frame_yuv420p->pts = frame_yuv420p->pkt_dts = av_rescale_q_rnd(
                frameCount,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_yuv420p->pkt_duration = av_rescale_q_rnd(
                1,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_yuv420p->pkt_pos = -1;

            t1 = getCurTime();
            ret = avcodec_send_frame(mBsPushStreamer->mVideoCodecCtx, frame_yuv420p);
            if (ret >= 0)
            {
                ret = avcodec_receive_packet(mBsPushStreamer->mVideoCodecCtx, pkt);
                if (ret >= 0)
                {
                    t2 = getCurTime();
                    encodeSuccessCount++;
                    
                    // 如果实际推流的是flv文件，不会执行里面的fix_packet_pts
                    if (pkt->pts == AV_NOPTS_VALUE)
                    {
                        std::cout << "pkt->pts == AV_NOPTS_VALUE\n";
                    }
                    pkt->stream_index = mBsPushStreamer->mVideoIndex;

                    pkt->pos = -1;
                    pkt->duration = frame_yuv420p->pkt_duration;

                    ret = mBsPushStreamer->writePkt(pkt);
                    if (ret < 0)
                    {
                        std::cout << fmt::format("writePkt : ret = {}\n", ret);
                    }
                }
                else
                {
                    // std::cout << fmt::format("avcodec_receive_packet error : ret = {}\n", ret);
                }
            }
            else
            {
                std::cout << fmt::format("avcodec_send_frame error : ret = {}\n", ret);
            }

            frameCount++;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    // std::cout << fmt::format("push_running is false!\n");
    // std::cout << fmt::format("end stream!\n");

    // av_write_trailer(mFmtCtx);  //写文件尾

    av_packet_unref(pkt);
    pkt = NULL;

    av_free(frame_yuv420p_buff);
    frame_yuv420p_buff = NULL;

    av_frame_free(&frame_yuv420p);
    // av_frame_unref(frame_yuv420p);
    frame_yuv420p = NULL;
}

int BsPushStreamer::writePkt(AVPacket* pkt) {
    mWritePkt_mtx.lock();
    int ret = av_write_frame(mFmtCtx, pkt);
    mWritePkt_mtx.unlock();

    return ret;

}

bool BsPushStreamer::getVideoFrame(VideoFrame *&frame, int &frameQSize)
{
    mRGB_VideoFrameQ_mtx.lock();

    if (!mRGB_VideoFrameQ.empty())
    {
        frame = mRGB_VideoFrameQ.front();
        mRGB_VideoFrameQ.pop();
        frameQSize = mRGB_VideoFrameQ.size();
        mRGB_VideoFrameQ_mtx.unlock();
        return true;
    }
    else
    {
        frameQSize = 0;
        mRGB_VideoFrameQ_mtx.unlock();
        return false;
    }
}

// void BsPushStreamer::pushVideoFrame(unsigned char *data, int width,int height)
void BsPushStreamer::stream(cv::Mat& image)
{
    
    int size = image.cols * image.rows * image.channels();
    // VideoFrame* frame = new VideoFrame(VideoFrame::BGR, image.cols, image.rows, size);
    memcpy(mVideoFrame->data, image.data, size);

    mRGB_VideoFrameQ_mtx.lock();
    nd_push_frame = true;
    // mRGB_VideoFrameQ.push(frame);
    mRGB_VideoFrameQ_mtx.unlock();
}

bool BsPushStreamer::videoFrameQisEmpty()
{
    return mRGB_VideoFrameQ.empty();
}

unsigned char BsPushStreamer::clipValue(unsigned char x, unsigned char min_val, unsigned char max_val)
{
    if (x > max_val) { return max_val; }
    else if (x < min_val) { return min_val; }
    else { return x; }
}

bool BsPushStreamer::bgr24ToYuv420p(unsigned char *bgrBuf, int w, int h, unsigned char *yuvBuf)
{

    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    memset(yuvBuf, 0, w * h * 3 / 2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w * h;
    ptrV = ptrU + (w * h * 1 / 4);
    unsigned char y, u, v, r, g, b;

    for (int j = 0; j < h; ++j)
    {

        ptrRGB = bgrBuf + w * j * 3;
        for (int i = 0; i < w; i++)
        {
            b = *(ptrRGB++);
            g = *(ptrRGB++);
            r = *(ptrRGB++);

            y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = clipValue(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0)
            {
                *(ptrU++) = clipValue(u, 0, 255);
            }
            else
            {
                if (i % 2 == 0)
                {
                    *(ptrV++) = clipValue(v, 0, 255);
                }
            }
        }
    }
    return true;
}
