#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
#include "avpacketqueue.h"
#include "thread.h"
#include <QObject>
class DemuxThread : public Thread {
    Q_OBJECT
public:
    DemuxThread(AVPacketQueue* audio_queue, AVPacketQueue* video_queue, QObject* parent = nullptr);
    ~DemuxThread();
    int Init(const char* url, AVStream** audio_stream, AVStream** video_stream, AVFormatContext** ifmt_ctx); // 一般在init去初始化，而不是在构造函数中初始化
    int Start() override;
    int Stop();
    void Run() override;

    AVCodecParameters* AudioCodecParameters();
    AVCodecParameters* VideoCodecParameters();
    AVFormatContext* ifmt_ctx_ = nullptr;
    int get_duration_time();
    bool pause = false;
    uint8_t* data_ = nullptr;

private:
    std::string url_; // 文件名
    char err2str[256]; // 用于存储错误信息
    AVPacketQueue* audio_queue_ = nullptr;
    AVPacketQueue* video_queue_ = nullptr;
    int audio_index_ = -1;
    int video_index_ = -1;
    std::mutex m_mutex;

public slots:
    void on_audioOutput(QIODevice* audioOuput, uint8_t* data, int data_size);
    void on_seek_frame(int ms);
};

#endif // DEMUXTHREAD_H
