#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include "avpacketqueue.h"
#include "qaudioplayer.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}
#include "avpacketqueue.h"
#include <QObject>

#include "thread.h"

class DecodeThread : public Thread {
    Q_OBJECT
public:
    DecodeThread(AVPacketQueue* packet_queue, AVStream* av_stream = nullptr, AVFormatContext* ifmt_ctx = nullptr, QObject* parent = nullptr);
    ~DecodeThread();
    int Init(AVCodecParameters* par);
    void SetAPlayer(QAudioPlayer* audio_player)
    {
        audio_player_ = audio_player;
    };
    int Start() override;
    int Stop();
    void Run() override;

    bool IsPause() { return pause_.load(); }
    bool IsPlay() { return !pause_.load(); }
    void Pause() { pause_.store(true); }
    void Play() { pause_.store(false); }

    int64_t startDTS = -1;
    int64_t startTime = -1;
    int current_frame_ms = -1;
    int samples_number = 0; // audio
    uint8_t* data = nullptr; // audio
private:
    QAudioPlayer* audio_player_;
    char err2str[256]; // 用于存储错误信息
    AVCodecContext* codec_ctx_ = nullptr;
    AVPacketQueue* packet_queue_ = nullptr;
    // AVFrameQueue* frame_queue_ = nullptr;
    AVStream* av_stream_;
    AVFormatContext* ifmt_ctx_;
    std::atomic<bool> pause_ = true;
    std::mutex m_mutex;
    AVFrame* RGBFrameFromYUV(AVFrame*);
    int AFrameToSwr(AVFrame* avFrame);
    int waitForReachPtsTime(AVFrame* frame);
    QString frame_type(AVPictureType pict_type)
    {
        if (pict_type == AV_PICTURE_TYPE_I)
            return "AV_PICTURE_TYPE_I";
        else if (pict_type == AV_PICTURE_TYPE_P)
            return "AV_PICTURE_TYPE_P";
        else if (pict_type == AV_PICTURE_TYPE_B)
            return "AV_PICTURE_TYPE_B";
        else
            return "unknow";
    }
public slots:
    void on_seek_frame(int ms);
signals:
    void getReadyFrame(AVFrame* frame);
    void getReadyFrameTime(int);
};

#endif // DECODETHREAD_H
