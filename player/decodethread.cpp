#include "decodethread.h"
#include "qdebug.h"
#include <thread>
DecodeThread::DecodeThread(AVPacketQueue* packet_queue, AVStream* av_stream, AVFormatContext* ifmt_ctx, QObject* parent)
    : packet_queue_(packet_queue)
    , av_stream_(av_stream)
    , ifmt_ctx_(ifmt_ctx)
{
}

DecodeThread::~DecodeThread()
{
    if (thread_) {
        Stop();
    }
    if (codec_ctx_)
        avcodec_close(codec_ctx_);
}

int DecodeThread::Init(AVCodecParameters* param)
{

    if (!param) {
        qDebug() << "Init par is null";
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(NULL); //	Allocate an AVCodecContext and set its fields to default values.

    int ret = ret = avcodec_parameters_to_context(codec_ctx_, param);
    if (ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        qDebug() << "avcodec_paramters_to_context failed, err2str: " << err2str;
        return -1;
    }

    const AVCodec* codec = avcodec_find_decoder(codec_ctx_->codec_id); // 找到解码器

    // if(!codec) {}

    qDebug() << codec->id;
    ret = avcodec_open2(codec_ctx_, codec, nullptr); // 打开解码器 -》codec_ctx_
    // if(ret<0){}

    return 0;
}

int DecodeThread::Start()
{

    thread_ = new std::thread(&DecodeThread::Run, this);
    // if(!thread_)
    return 0;
}

int DecodeThread::Stop()
{
    return Thread::Stop();
}

void DecodeThread::Run()
{
    int ret;
    while (abort_ != 1) {
        if (IsPause()) {
            av_usleep(15000);
            continue;
        }
        if (packet_queue_->Size() == 0 && packet_queue_->IsEOF()) {
            avcodec_send_packet(codec_ctx_, nullptr);

            //>发送空指针给avcodec_send_packet解包
        } else if (packet_queue_->Size() > 0) {
            AVPacket* pkt = packet_queue_->Pop(3); // 10ms
            // qDebug() << "packet_queue_->Size() =" << packet_queue_->Size();

            ret = avcodec_send_packet(codec_ctx_, pkt);

            if (ret != 0) {
                qDebug() << "avcodec_send_packet fail";
            }
            av_packet_free(&pkt);
            // if(ret<0)

            // qDebug() << "一帧";
        } else {
            qDebug() << ("packet_queue_ is empty");
            av_usleep(50000);
            continue;
        }
        while (true) { // 可能有多帧
            int frame_time = 0;
            AVFrame* frame = av_frame_alloc();
            ret = avcodec_receive_frame(codec_ctx_, frame); // codec_ctx_ -》 frame

            if (ret == 0) { //
                // qDebug() << "frame_type:" << frame_type(frame->pict_type);
                // qDebug() << "frame_pkt_pos:" << frame->pkt_pos;
                if (codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) {
                    // qDebug() << "音频";
                    // qDebug() << audio_player_->audio_stream_->codecpar;
                    int data_size = 0;
                    data_size = AFrameToSwr(frame);
                    current_frame_ms = waitForReachPtsTime(frame);

                    // qDebug() << "a frame_time:" << frame_time;
                    audio_player_->Play(data, data_size);

                } else {
                    AVFrame* rgbFrame = nullptr;
                    // rgbFrame = RGBFrameFromYUV(frame);
                    current_frame_ms = waitForReachPtsTime(frame);
                    qDebug() << "v frame_time:" << current_frame_ms;
                    emit getReadyFrameTime(current_frame_ms);
                    if (rgbFrame) {
                        // frame_queue_->Push(rgbFrame); // AVFrameQueue::Push()
                        emit getReadyFrame(rgbFrame);

                    } else {
                        emit getReadyFrame(frame);
                        // frame_queue_->Push(frame);
                    }
                }
                continue;
            } else if (ret == AVERROR(EAGAIN)) {
                av_frame_unref(frame); // 如果没有获取解码帧失败，就把帧释放
                // av_strerror(ret, err2str, sizeof(err2str));
                // qDebug() << "avcodec_receive_frame failed, err2str: " << ret << err2str;
                break;
            } else {
                qDebug() << ret;
                av_strerror(ret, err2str, sizeof(err2str));
                abort_ = 1;
                qDebug() << "avcodec_receive_frame failed, err2str: " << err2str;
                break;
            }
        }
    }
}

AVFrame* DecodeThread::RGBFrameFromYUV(AVFrame* yuvFrame)
{
    if (yuvFrame->height == 0)
        return nullptr;
    if ((AVPixelFormat)yuvFrame->format != AVPixelFormat::AV_PIX_FMT_YUV420P)
        return nullptr;
    int ret = 0;
    AVFrame* rgbFrame = av_frame_alloc(); // 这是用来存储 RGB 数据的帧

    // 复制 yuvFrame 的所有信息，包括 dts、pts 等
    ret = av_frame_copy_props(rgbFrame, yuvFrame);
    if (ret < 0) {
        qDebug() << "Failed to copy properties from yuvFrame";

        return nullptr;
    }

    // 设置 RGB 输出帧的尺寸和像素格式
    int width = yuvFrame->width;
    int height = yuvFrame->height;
    rgbFrame->format = AV_PIX_FMT_RGB24; // 你可以选择 RGB24 或 RGBA 格式
    rgbFrame->width = width;
    rgbFrame->height = height;

    ret = av_frame_get_buffer(rgbFrame, 32); // 分配内存 ，占用24，但是要内存对齐
    if (ret != 0) {
        qDebug() << "av_frame_get_buffer fail";
        return nullptr;
    }

    // 使用 swscale 进行颜色空间转换
    struct SwsContext* sws_ctx = sws_getContext(
        width, height, (AVPixelFormat)yuvFrame->format,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        fprintf(stderr, "Failed to create SwsContext\n");
        return nullptr;
    }

    ret = sws_scale(sws_ctx, yuvFrame->data, yuvFrame->linesize, 0, height, rgbFrame->data, rgbFrame->linesize);

    if (ret <= 0) {
        qDebug() << "sws_scale fail";
        return nullptr;
    }

    return rgbFrame;
}

int DecodeThread::AFrameToSwr(AVFrame* avFrame)
{
    SwrContext* swrCtx = nullptr;

    if (!avFrame)
        return 0;

    AVChannelLayout channelLayout = avFrame->ch_layout;
    AVSampleFormat inSampleFormat = static_cast<AVSampleFormat>(avFrame->format);
    AVSampleFormat outSampleFormat = av_get_packed_sample_fmt(static_cast<AVSampleFormat>(avFrame->format));

    int res = swr_alloc_set_opts2(&swrCtx,
        &channelLayout, outSampleFormat, avFrame->sample_rate,
        &channelLayout, inSampleFormat, avFrame->sample_rate,
        0, nullptr);
    if (res < 0)
        return 0;

    if (swr_init(swrCtx) == 0) {
        int out_count = swr_get_out_samples(swrCtx, avFrame->nb_samples) + 128;
        samples_number = av_samples_get_buffer_size(nullptr, channelLayout.nb_channels, out_count, outSampleFormat, 0); // int size
        data = (uint8_t*)av_malloc(samples_number); // uint8_t *data

        int out_samples = swr_convert(swrCtx, &data, out_count, (const uint8_t**)avFrame->data, avFrame->nb_samples);
        samples_number = av_samples_get_buffer_size(nullptr, channelLayout.nb_channels, out_samples, outSampleFormat, 0);
    }
    if (swrCtx)
        swr_free(&swrCtx);

    return samples_number;
}

int DecodeThread::waitForReachPtsTime(AVFrame* frame)
{
    AVRational time_base = av_stream_->time_base; // 帧速率，固定的值
    AVRational time_base_q = { 1, AV_TIME_BASE };
    int64_t dts_time = av_rescale_q(frame->pkt_dts, time_base, time_base_q);
    // qDebug() << time_base.num << " " << time_base.den;
    if (frame->height > 1) {
        qDebug() << frame->pts << " " << dts_time;
    }

    if (startTime < 0) { //(startTime < 0意味着还未开始解码第一帧
        startDTS = dts_time; // 第一帧的时间偏移，microseconds
        startTime = av_gettime(); // Get the current time in microseconds.
        qDebug() << startDTS << startTime;
        printf("startDTS= %lld, startTime= %lld", startDTS, startTime);
    } else {
        int64_t nowTime = av_gettime() - startTime;
        // qDebug() << dts_time - startDTS - nowTime;
        if ((dts_time - startDTS) > nowTime)

            av_usleep(dts_time - startDTS - nowTime);
    }
    return dts_time / 1000;
}

void DecodeThread::on_seek_frame(int ms)
{
    // play_state = 0;
    // std::lock_guard<std::mutex> lock(m_mutex);
    printf("%lld += %d - %d", startTime, ms, current_frame_ms);
    startTime += ms - (current_frame_ms);

    avcodec_flush_buffers(codec_ctx_);
    // std::lock_guard<std::mutex> unlock(m_mutex);
    //  play_state = 1;
}
