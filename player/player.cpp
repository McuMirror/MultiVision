extern "C" {
#include "libavutil/avutil.h"
}

#include "decodethread.h"
#include "demuxthread.h"
#include "player.h"
#include "qaudioplayer.h"

#include <QApplication>
#include <QFileDialog>
#include <QMediaDevices>
#include <QQueue>
#include <thread>
// #include <windows.h>
Player::Player(QWidget* parent)
    : QObject(parent)

{

    /*
  解包线程：需要视频文件

  音频解码线程：需要包队列，音频帧队列
  视频解码线程：需要包队列，视频帧队列
  ——api:
    Init()
    start()


  音频播放线程：需要音频帧队列，播放器
  视频播放线程：需要视频帧队列，播放器
  ——api:
    Init()
    start()

抽象为几个动作：
1、选择视频
2、播放
3、暂停
4、选择位置
*/
    ;
    printf_s("hello ffmpeg, version is%s\n", av_version_info());

    qWarning() << av_version_info();

    // qDebug() << "SDL_Init(SDL_INIT_AUDIO): " << SDL_Init(SDL_INIT_AUDIO);
    /*
        AVPacketQueue audio_packet_queue;
        AVPacketQueue video_packet_queue;

        AVFrameQueue audio_frame_queue;
        AVFrameQueue video_frame_queue;

        AVStream* video_stream = nullptr;
        AVStream* audio_stream = nullptr;

        DemuxThread* demux_thread;
        DecodeThread* video_decode_thread;
        DecodeThread* audio_decode_thread;

        QAudioPlayer* audio_ouput;
    */

    qDebug() << "MainWindow Init Finished";
}

Player::~Player()
{
    demux_thread->Stop();
    delete demux_thread;

    audio_decode_thread->Stop();
    delete audio_decode_thread;

    video_decode_thread->Stop();
    delete video_decode_thread;

    delete audio_ouput;
}
int Player::initialization(QString videoPath)
{
    int ret = 0;
    // 解包分为两个线程

    // 杜蕾斯线程的run循环是在新建线程运行的
    demux_thread = new DemuxThread(&audio_packet_queue, &video_packet_queue);

    ret = demux_thread->Init(videoPath.toStdString().c_str(), &audio_stream, &video_stream, &ifmt_ctx);

    //  视频解码：packet_queue -》 frame_queue
    video_decode_thread = new DecodeThread(&video_packet_queue, video_stream, ifmt_ctx);
    ret = video_decode_thread->Init(demux_thread->VideoCodecParameters());

    // 音频解码：packet_queue -》 frame_queue
    audio_decode_thread = new DecodeThread(&audio_packet_queue, audio_stream, ifmt_ctx);
    ret = audio_decode_thread->Init(demux_thread->AudioCodecParameters());

    if (ret)
        qDebug() << "audio_decode_thread->Start() fail";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    audio_ouput = new QAudioPlayer(audio_stream);
    audio_ouput->Init();

    audio_decode_thread->SetAPlayer(audio_ouput);

    // 把解码的帧发射出去
    connect(
        video_decode_thread, &DecodeThread::getReadyFrame, this, [&](std::shared_ptr<AVFrame> frame) { emit getReadyFrame(frame); }, Qt::QueuedConnection);
    // 发送音频信号到播放函数
    connect(audio_ouput, &QAudioPlayer::audioOutput_sig, demux_thread, &DemuxThread::on_audioOutput, Qt::QueuedConnection);

    installEventFilter(this);
    qDebug() << "duration=" << demux_thread->get_duration_time();

    int maximum = demux_thread->get_duration_time() / 1000.0;

    qDebug() << "width=" << demux_thread->VideoCodecParameters()->width << ",height = " << demux_thread->VideoCodecParameters()->height;
    emit playerInitReady(demux_thread->VideoCodecParameters()->width, demux_thread->VideoCodecParameters()->height);

    IsInit = true;
    qDebug() << "Player Init finished :" << videoPath;
    return 0;
}

void Player::play()
{

    // 如果未初始化，尝试初始化一次。
    if (!IsInit) {
        initialization(video);
        return;
    }
    //  还未开始->刚开始
    //  暂停 ->恢复播放
    //  已经在播放
    if (audio_decode_thread->IsPause() && video_decode_thread->IsPause()) {
        if (audio_decode_thread->startTime == -1 && video_decode_thread->startTime == -1) {

            demux_thread->Start();
            audio_decode_thread->Start();
            video_decode_thread->Start();
        } else if (pause_time >= 0) {
            audio_decode_thread->startTime += av_gettime() - pause_time;
            video_decode_thread->startTime += av_gettime() - pause_time;
            pause_time = -1;
        }

        audio_decode_thread->Play();
        video_decode_thread->Play();
    }
}

void Player::pause()
{

    if (audio_decode_thread->IsPlay() && video_decode_thread->IsPlay()) { // 播放转暂停
        audio_decode_thread->Pause();
        video_decode_thread->Pause();
        pause_time = av_gettime(); // 记录暂停时间
    }
}
