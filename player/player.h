#ifndef PLAYER_H
#define PLAYER_H

// #include "sdlvideoplayer.h"
#include <QMainWindow>

#include "decodethread.h"
#include "demuxthread.h"

// #include "qaudioplayer.h"

// #include "videoplaythread.h"
#include <QApplication>
#include <QMediaDevices>
#include <QQueue>
#include <thread>
// #include <windows.h>

class Player : public QObject {
    Q_OBJECT

public:
    Player(QWidget* parent = nullptr);
    ~Player();
    // void InitOpenglTexture();
    AVPacketQueue audio_packet_queue;
    AVPacketQueue video_packet_queue;

    //    AVFrameQueue audio_frame_queue;
    //    AVFrameQueue video_frame_queue;

    AVFormatContext* ifmt_ctx = nullptr;
    AVStream* video_stream = nullptr;
    AVStream* audio_stream = nullptr;

    DemuxThread* demux_thread = nullptr;
    DecodeThread* video_decode_thread = nullptr;
    DecodeThread* audio_decode_thread = nullptr;

    QAudioPlayer* audio_ouput = nullptr;

    unsigned int pause_time = -1;

    bool IsInit = false;

signals:
    void getReadyFrame(AVFrame* frame);
    void playerInitReady(int w, int h);

public:
    int initialization(QString videoPath);
    void play();
    void pause();
    // int getState(); //-1 未初始化 ； 0已初始化、停止；1已初始化、播放

private slots:
    //    void on_pushPlay_clicked();
    //    void on_pushPause_clicked();
    //    void on_pushOpen_clicked();

private:
    QString video
        = "D:\\Qt\\ffmpeg_sdl_player_v2\\ffmpeg_sdl_player\\10_minu11tes.mp4";
};
#endif // PLAYER_H
