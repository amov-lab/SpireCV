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

static int set_hwframe_ctx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx, int width, int height)
{
    AVBufferRef *hw_frames_ref;
    AVHWFramesContext *frames_ctx = NULL;
    int err = 0;

    if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx)))
    {
        fprintf(stderr, "Failed to create VAAPI frame context.\n");
        return -1;
    }
    frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
    frames_ctx->format = AV_PIX_FMT_VAAPI;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->width = width;
    frames_ctx->height = height;
    frames_ctx->initial_pool_size = 20;
    if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0)
    {
        // fprintf(stderr, "Failed to initialize VAAPI frame context."
        //         "Error code: %s\n",av_err2str(err));
        av_buffer_unref(&hw_frames_ref);
        return err;
    }
    ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    if (!ctx->hw_frames_ctx)
        err = AVERROR(ENOMEM);

    av_buffer_unref(&hw_frames_ref);
    return err;
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
    err = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI,
                                 NULL, NULL, 0);

    AVCodec *videoCodec = avcodec_find_encoder_by_name(encoder.c_str());
    if (!videoCodec)
    {
        // std::cout << fmt::format("Using encoder:[{}] error!\n", encoder);
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
    mVideoCodecCtx->pix_fmt = AV_PIX_FMT_VAAPI;
    mVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    mVideoCodecCtx->width = width;
    mVideoCodecCtx->height = height;
    mVideoCodecCtx->time_base = {1, fps};
    mVideoCodecCtx->framerate = {fps, 1};
    mVideoCodecCtx->gop_size = 12;
    mVideoCodecCtx->max_b_frames = 0;
    mVideoCodecCtx->thread_count = 1;
    mVideoCodecCtx->sample_aspect_ratio = (AVRational){1, 1};

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
    if ((err = set_hwframe_ctx(mVideoCodecCtx, hw_device_ctx, width, height)) < 0)
    {
        std::cout << "set_hwframe_ctx error\n";
        return false;
    }

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
        if (ret < 0)
        {
            // std::cout << fmt::format("avio_open error url: {}\n", name.c_str());
            // std::cout << fmt::format("ret = {} : {}\n", ret, av_err2str(ret));
            // std::cout << fmt::format("ret = {}\n", ret);
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

void BsPushStreamer::encodeVideoAndWriteStreamThread(void *arg)
{
    // PushExecutor *executor = (PushExecutor *)arg;
    BsPushStreamer *mBsPushStreamer = (BsPushStreamer *)arg;
    int width = mBsPushStreamer->mVideoFrame->width;
    int height = mBsPushStreamer->mVideoFrame->height;

    // 未编码的视频帧（bgr格式）
    // VideoFrame *videoFrame = NULL;
    // 未编码视频帧队列当前长度
    // int videoFrameQSize = 0;
    AVFrame *hw_frame = NULL;
    AVFrame *frame_nv12 = av_frame_alloc();
    frame_nv12->format = AV_PIX_FMT_NV12;
    frame_nv12->width = width;
    frame_nv12->height = height;

    int frame_nv12_buff_size = av_image_get_buffer_size(AV_PIX_FMT_NV12, width, height, 1);
    uint8_t *frame_nv12_buff = (uint8_t *)av_malloc(frame_nv12_buff_size);
    av_image_fill_arrays(
        frame_nv12->data, frame_nv12->linesize,
        frame_nv12_buff,
        AV_PIX_FMT_NV12,
        width, height, 1);

    if (!(hw_frame = av_frame_alloc()))
    {
        std::cout << "Error while av_frame_alloc().\n";
    }
    if (av_hwframe_get_buffer(mBsPushStreamer->mVideoCodecCtx->hw_frames_ctx, hw_frame, 0) < 0)
    {
        std::cout << "Error while av_hwframe_get_buffer.\n";
    }
    if (!hw_frame->hw_frames_ctx)
    {
        std::cout << "Error while hw_frames_ctx.\n";
    }

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
            // frame_bgr 转  frame_nv12
            //mBsPushStreamer->bgr24ToYuv420p(mBsPushStreamer->mVideoFrame->data, width, height, frame_nv12_buff);
            mBsPushStreamer->Rgb2NV12(mBsPushStreamer->mVideoFrame->data, 3, width, height, frame_nv12_buff);

            frame_nv12->pts = frame_nv12->pkt_dts = av_rescale_q_rnd(
                frameCount,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_nv12->pkt_duration = av_rescale_q_rnd(
                1,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            frame_nv12->pkt_pos = -1;

                 hw_frame->pts = hw_frame->pkt_dts = av_rescale_q_rnd(
                frameCount,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            hw_frame->pkt_duration = av_rescale_q_rnd(
                1,
                mBsPushStreamer->mVideoCodecCtx->time_base,
                mBsPushStreamer->mVideoStream->time_base,
                (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            hw_frame->pkt_pos = -1;

            if (av_hwframe_transfer_data(hw_frame, frame_nv12, 0) < 0)
            {
                std::cout << "Error while transferring frame data to surface.\n";
            }

            t1 = getCurTime();
            ret = avcodec_send_frame(mBsPushStreamer->mVideoCodecCtx, hw_frame);
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
                    pkt->duration = frame_nv12->pkt_duration;

                    ret = mBsPushStreamer->writePkt(pkt);
                    if (ret < 0)
                    {
                        // std::cout << fmt::format("writePkt : ret = {}\n", ret);
                    }
                }
                else
                {
                    // std::cout << fmt::format("avcodec_receive_packet error : ret = {}\n", ret);
                }
            }
            else
            {
                // std::cout << fmt::format("avcodec_send_frame error : ret = {}\n", ret);
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

    av_free(frame_nv12_buff);
    frame_nv12_buff = NULL;

    av_frame_free(&frame_nv12);
    av_frame_unref(hw_frame);
    frame_nv12 = NULL;
    hw_frame = NULL;
}

int BsPushStreamer::writePkt(AVPacket *pkt)
{
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
void BsPushStreamer::stream(cv::Mat &image)
{

    int size = image.cols * image.rows * image.channels();
   //VideoFrame *frame = new VideoFrame(VideoFrame::BGR, image.cols, image.rows);
    cv::Mat bgr = cv::Mat::zeros(image.size(), CV_8UC3);
    cv::cvtColor(image, bgr, cv::COLOR_BGR2RGB);

    memcpy(mVideoFrame->data, bgr.data, size);

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
    if (x > max_val)
    {
        return max_val;
    }
    else if (x < min_val)
    {
        return min_val;
    }
    else
    {
        return x;
    }
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

// https://software.intel.com/en-us/node/503873
// YCbCr Color Model:
//     The YCbCr color space is used for component digital video and was developed as part of the ITU-R BT.601 Recommendation. YCbCr is a scaled and offset version of the YUV color space.
//     The Intel IPP functions use the following basic equations [Jack01] to convert between R'G'B' in the range 0-255 and Y'Cb'Cr' (this notation means that all components are derived from gamma-corrected R'G'B'):
//     Y' = 0.257*R' + 0.504*G' + 0.098*B' + 16
//     Cb' = -0.148*R' - 0.291*G' + 0.439*B' + 128
//     Cr' = 0.439*R' - 0.368*G' - 0.071*B' + 128

// Y' = 0.257*R' + 0.504*G' + 0.098*B' + 16
static float Rgb2Y(float r0, float g0, float b0)
{
    float y0 = 0.257f * r0 + 0.504f * g0 + 0.098f * b0 + 16.0f;
    return y0;
}

// U equals Cb'
// Cb' = -0.148*R' - 0.291*G' + 0.439*B' + 128
static float Rgb2U(float r0, float g0, float b0)
{
    float u0 = -0.148f * r0 - 0.291f * g0 + 0.439f * b0 + 128.0f;
    return u0;
}

// V equals Cr'
// Cr' = 0.439*R' - 0.368*G' - 0.071*B' + 128
static float Rgb2V(float r0, float g0, float b0)
{
    float v0 = 0.439f * r0 - 0.368f * g0 - 0.071f * b0 + 128.0f;
    return v0;
}

// Convert two rows from RGB to two Y rows, and one row of interleaved U,V.
// I0 and I1 points two sequential source rows.
// I0 -> rgbrgbrgbrgbrgbrgb...
// I1 -> rgbrgbrgbrgbrgbrgb...
// Y0 and Y1 points two sequential destination rows of Y plane.
// Y0 -> yyyyyy
// Y1 -> yyyyyy
// UV0 points destination rows of interleaved UV plane.
// UV0 -> uvuvuv
static void Rgb2NV12TwoRows(const unsigned char I0[],
                            const unsigned char I1[],
                            int step,
                            const int image_width,
                            unsigned char Y0[],
                            unsigned char Y1[],
                            unsigned char UV0[])
{
    int x; // Column index

    // Process 4 source pixels per iteration (2 pixels of row I0 and 2 pixels of row I1).
    for (x = 0; x < image_width; x += 2)
    {
        // Load R,G,B elements from first row (and convert to float).
        float r00 = (float)I0[x * step + 0];
        float g00 = (float)I0[x * step + 1];
        float b00 = (float)I0[x * step + 2];

        // Load next R,G,B elements from first row (and convert to float).
        float r01 = (float)I0[x * step + step + 0];
        float g01 = (float)I0[x * step + step + 1];
        float b01 = (float)I0[x * step + step + 2];

        // Load R,G,B elements from second row (and convert to float).
        float r10 = (float)I1[x * step + 0];
        float g10 = (float)I1[x * step + 1];
        float b10 = (float)I1[x * step + 2];

        // Load next R,G,B elements from second row (and convert to float).
        float r11 = (float)I1[x * step + step + 0];
        float g11 = (float)I1[x * step + step + 1];
        float b11 = (float)I1[x * step + step + 2];

        // Calculate 4 Y elements.
        float y00 = Rgb2Y(r00, g00, b00);
        float y01 = Rgb2Y(r01, g01, b01);
        float y10 = Rgb2Y(r10, g10, b10);
        float y11 = Rgb2Y(r11, g11, b11);

        // Calculate 4 U elements.
        float u00 = Rgb2U(r00, g00, b00);
        float u01 = Rgb2U(r01, g01, b01);
        float u10 = Rgb2U(r10, g10, b10);
        float u11 = Rgb2U(r11, g11, b11);

        // Calculate 4 V elements.
        float v00 = Rgb2V(r00, g00, b00);
        float v01 = Rgb2V(r01, g01, b01);
        float v10 = Rgb2V(r10, g10, b10);
        float v11 = Rgb2V(r11, g11, b11);

        // Calculate destination U element: average of 2x2 "original" U elements.
        float u0 = (u00 + u01 + u10 + u11) * 0.25f;

        // Calculate destination V element: average of 2x2 "original" V elements.
        float v0 = (v00 + v01 + v10 + v11) * 0.25f;

        // Store 4 Y elements (two in first row and two in second row).
        Y0[x + 0] = (unsigned char)(y00 + 0.5f);
        Y0[x + 1] = (unsigned char)(y01 + 0.5f);
        Y1[x + 0] = (unsigned char)(y10 + 0.5f);
        Y1[x + 1] = (unsigned char)(y11 + 0.5f);

        // Store destination U element.
        UV0[x + 0] = (unsigned char)(u0 + 0.5f);

        // Store destination V element (next to stored U element).
        UV0[x + 1] = (unsigned char)(v0 + 0.5f);
    }
}

// Convert image I from pixel ordered RGB to NV12 format.
// I - Input image in pixel ordered RGB format
// image_width - Number of columns of I
// image_height - Number of rows of I
// J - Destination "image" in NV12 format.

// I is pixel ordered RGB color format (size in bytes is image_width*image_height*3):
// RGBRGBRGBRGBRGBRGB
// RGBRGBRGBRGBRGBRGB
// RGBRGBRGBRGBRGBRGB
// RGBRGBRGBRGBRGBRGB
//
// J is in NV12 format (size in bytes is image_width*image_height*3/2):
// YYYYYY
// YYYYYY
// UVUVUV
// Each element of destination U is average of 2x2 "original" U elements
// Each element of destination V is average of 2x2 "original" V elements
//
// Limitations:
// 1. image_width must be a multiple of 2.
// 2. image_height must be a multiple of 2.
// 3. I and J must be two separate arrays (in place computation is not supported).
void BsPushStreamer::Rgb2NV12(const unsigned char I[], int step,
                              const int image_width,
                              const int image_height,
                              unsigned char J[])
{
    // In NV12 format, UV plane starts below Y plane.
    unsigned char *UV = &J[image_width * image_height];

    // I0 and I1 points two sequential source rows.
    const unsigned char *I0; // I0 -> rgbrgbrgbrgbrgbrgb...
    const unsigned char *I1; // I1 -> rgbrgbrgbrgbrgbrgb...

    // Y0 and Y1 points two sequential destination rows of Y plane.
    unsigned char *Y0; // Y0 -> yyyyyy
    unsigned char *Y1; // Y1 -> yyyyyy

    // UV0 points destination rows of interleaved UV plane.
    unsigned char *UV0; // UV0 -> uvuvuv

    int y; // Row index

    // In each iteration: process two rows of Y plane, and one row of interleaved UV plane.
    for (y = 0; y < image_height; y += 2)
    {
        I0 = &I[y * image_width * step]; // Input row width is image_width*3 bytes (each pixel is R,G,B).
        I1 = &I[(y + 1) * image_width * step];

        Y0 = &J[y * image_width]; // Output Y row width is image_width bytes (one Y element per pixel).
        Y1 = &J[(y + 1) * image_width];

        UV0 = &UV[(y / 2) * image_width]; // Output UV row - width is same as Y row width.

        // Process two source rows into: Two Y destination row, and one destination interleaved U,V row.
        Rgb2NV12TwoRows(I0,
                        I1,
                        step,
                        image_width,
                        Y0,
                        Y1,
                        UV0);
    }
}
