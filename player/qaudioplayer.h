#ifndef QAUDIOPLAYER_H
#define QAUDIOPLAYER_H

#include <QObject>
#ifdef __cplusplus ///
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
#include "libswresample/swresample.h"
}
#endif
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioSink>
#include <mutex>
class QAudioPlayer : public QObject {
    Q_OBJECT
public:
    QAudioPlayer(AVStream* audio_stream, QObject* parent = nullptr);
    ~QAudioPlayer();
    // int Init(AVFrameQueue* frame_queue, AVStream* audio_stream);
    int Init();
    void Play(uint8_t* data, int data_size);
    // const char* GetData() { return reinterpret_cast<const char*>(data); };
    // int GetSize() { return samples_number; };
    AVStream* audio_stream_ = nullptr;

private:
    QAudioSink* audioSink_ = nullptr;
    QIODevice* audioOutput_ = nullptr; // 这个指针最终要指向QAudioSink::start
    std::mutex m_mutex;

signals:
    void audioOutput_sig(QIODevice* audioOuput, uint8_t* data, int data_size);
};

#endif // QAUDIOPLAYER_H
