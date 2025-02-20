#include "qaudioplayer.h"

#include "qapplication.h"
#include "qdebug.h"

#include <QBuffer>
#include <QByteArray>
#include <QIODevice>
#include <QMediaDevices>
#include <QThread>

QAudioPlayer::QAudioPlayer(AVStream* audio_stream, QObject* parent)
    : audio_stream_(audio_stream)
{
    qDebug() << "QAudioPlayer()";
    QThread* currentThread = QThread::currentThread();
    qDebug() << "this_thread::get_id()" << currentThread;
}

QAudioPlayer::~QAudioPlayer()
{
    if (!audioSink_->isNull())
        delete audioSink_;
}

// int QAudioPlayer::Init(AVFrameQueue *frame_queue, AVStream *audio_stream)
//{

//}

/*
 * 简单来说，配置一个QAudioDevice对象，并将QAudioSink挂在上面
 */
int QAudioPlayer::Init()
{

    QAudioFormat format;
    AVCodecParameters* codecpar = audio_stream_->codecpar;

    AVSampleFormat sample_fmt = static_cast<AVSampleFormat>(codecpar->format); // 采样格式

    switch (sample_fmt) {
    case AV_SAMPLE_FMT_U8P:
    case AV_SAMPLE_FMT_U8:
        format.setSampleFormat(QAudioFormat::UInt8);
        break;
    case AV_SAMPLE_FMT_S16P:
    case AV_SAMPLE_FMT_S16:
        format.setSampleFormat(QAudioFormat::Int16);
        break;
    case AV_SAMPLE_FMT_S32P:
    case AV_SAMPLE_FMT_S32:
        format.setSampleFormat(QAudioFormat::Int32);
        break;
    case AV_SAMPLE_FMT_FLTP:
    case AV_SAMPLE_FMT_FLT:
        format.setSampleFormat(QAudioFormat::Float);
        break;
    default:
        qWarning("Unsupported audio format");
        return -1;
    }

    format.setSampleRate(codecpar->sample_rate); // 设置采样率
    format.setChannelCount(codecpar->ch_layout.nb_channels); // 双声道

    format.setChannelConfig(QAudioFormat::ChannelConfigStereo); // 声道配置

    // QAudioOutput* audioOutput = new QAudioOutput(QAudioDevice::defaultOutputDevice, format);

    //    if (!QAudioDevice::defaultOutputDevice().isFormatSupported(format)) {
    //        qWarning() << "指定的音频格式不被支持，无法播放音频！";
    //        return -1;
    //
    QAudioDevice defaultAudioOutput(QMediaDevices::defaultAudioOutput());

    if (format.isValid()) {
        audioSink_ = new QAudioSink(defaultAudioOutput, format);

        QAudioFormat audioOutputFormat = audioSink_->format();
        QString msg = QString("Check output audio device format (Qt):\n"
                              "- number of channels - %1\n"
                              "- sample rate - %2\n")
                          .arg(audioOutputFormat.channelCount())
                          .arg(audioOutputFormat.sampleRate());
        qDebug() << msg;
        audioOutput_ = audioSink_->start();
        // 播放解码后的音频数据
    } else {
        qDebug() << format;
        return -1;
    }

    return 0;
}

void QAudioPlayer::Play(uint8_t* data, int data_size)
{

    if (!audioOutput_->isOpen() || !audioOutput_->isWritable()) {
        fprintf(stderr, "Audio output device is not ready for writing\n");
        return;
    }
#if false
    audioOutput_->write(reinterpret_cast<const char*>(data), data_size);
    // qDebug() << data_size;
#else
    emit audioOutput_sig(audioOutput_, (data), data_size);
#endif
    //  ----------------------------------------------
}
