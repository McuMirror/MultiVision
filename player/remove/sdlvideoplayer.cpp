#include "sdlvideoplayer.h"
#include <QDebug>
SDLVideoPlayer::SDLVideoPlayer()
{
}

SDLVideoPlayer::~SDLVideoPlayer()
{
    free(yuv_buf_);
    qDebug() << "~SDLVideoPlayer()";
}

int SDLVideoPlayer::setVideo(AVFrameQueue* frame_queue, AVStream* video_stream)
{
    frame_queue_ = frame_queue;
    video_stream_ = (video_stream);
    if (!video_stream_)
        qDebug() << "*_stream_ nullptr";
    else {
        qDebug() << video_stream->start_time;
        qDebug() << av_gettime();
        time_start_base = av_gettime() + video_stream->start_time * 0;
        video_width_ = video_stream->codecpar->width;
        video_height_ = video_stream->codecpar->height;
    }
    return 0;
}

SDL_Window* SDLVideoPlayer::InitWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return nullptr;
    }
    qDebug() << "SDL_CreateWindow ...";

    win_ = SDL_CreateWindow("SDLPlayer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, video_width_, video_height_, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win_) {
        qDebug() << "SDL_CreateWindow fail";
        return nullptr;
    }
    return win_;
}

SDL_Window* SDLVideoPlayer::InitWindow(const void* data)
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return nullptr;
    }
    qDebug() << "SDL_CreateWindow ...";
    win_ = SDL_CreateWindowFrom(data);

    if (!win_) {
        qDebug() << "SDL_CreateWindow fail";
        return nullptr;
    }
    return win_;
}
int SDLVideoPlayer::InitTexture()
{
    qDebug() << "SDLVideoPlayer::Init() ...";

    renderer_ = SDL_CreateRenderer(win_, -1, 0);
    if (!renderer_) {
        return -1;
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width_, video_height_);
    if (!texture_) {
        qDebug() << "SDL_CreateTexture fail";
    }

    yuv_buf_size_ = video_width_ * video_height_ * 1.5;
    yuv_buf_ = (uint8_t*)malloc(yuv_buf_size_);
    //  eventThread = new std::thread(&SDLVideoPlayer::pollEvents, this);
    //   faild:
    //    释放资源
    qDebug() << "SDLVideoPlayer::Init() success";
    return 0;
}

int SDLVideoPlayer::Play()
{
    qDebug() << "Play() running=" << running;
    while (running) {

        // qDebug() << "frame_queue_->Size(): " << frame_queue_->Size();
        AVFrame* frame = frame_queue_->Front();

        videoRefresh(frame);

        frame = frame_queue_->Pop(5);

        if (frame)
            av_frame_free(&frame);
    }
    qDebug() << "Play finished";
    // std::thread meventThread(pollEvents);

    // SDL_Event event;

    // 看这里 - - - - - 关键 - - -
    // RefreshLoopWaitEvent(&event);
    // 读取事件 - - - - - - - -
    //    switch (event.type) {
    //    case SDL_KEYDOWN:
    //        if (event.key.keysym.sym == SDLK_ESCAPE) {
    //            qDebug() << ("esc key down");
    //            return 0;
    //        }
    //        break;
    //    case SDL_QUIT:
    //        return 0;
    //    default:
    //        break;
    //    }

    return 0;
}
void SDLVideoPlayer::pollEvents()
{

    while (running) {
        SDL_Event event;
        //  qDebug() << "SDL_Event event;";
        while (SDL_PollEvent(&event)) {
            // qDebug() << "while (SDL_PollEvent(&event))";
            if (event.type == SDL_QUIT) {
                running = false; // 退出时停止所有线程
            } else if (event.type == SDL_WINDOWEVENT) {

                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // 获取新的窗口尺寸
                    qDebug() << "event.type == SDL_WINDOWEVENT";
                    int windowWidth = event.window.data1;
                    int windowHeight = event.window.data2;
                    SDL_Rect new_rect = { 0, 0, windowWidth, windowHeight };
                    display_rect_.store(new_rect, std::memory_order_release);
                    SDL_DestroyTexture(texture_);

                    //  SDL_RenderSetViewport(renderer_, &new_rect);
                }
            }
        }
    }
}

void SDLVideoPlayer::simpleRun()
{
    int ret = 0;
    ret = InitTexture();
    // if (ret != 0)

    play_thread = std::make_unique<std::thread>(&SDLVideoPlayer::Play, this);
    // SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
    pollEvents();
    qDebug() << "simpleRun leave";
    // delete play_thread;
}
#define REFRESH_RATE 0.00
void SDLVideoPlayer::RefreshLoopWaitEvent(SDL_Event* event)
{

    // SDL_PumpEvents();
    // if (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {

        // remaining_time = REFRESH_RATE;
        AVFrame* frame = frame_queue_->Front();
        videoRefresh(frame);
        frame = frame_queue_->Pop(1);
        if (frame)
            av_frame_free(&frame);
        // SDL_PollEvent(event);
        // SDL_PumpEvents();
    }
}
void SDLVideoPlayer::waitForReachPtsTime(AVFrame* frame)
{

    AVRational time_base = video_stream_->time_base;
    AVRational time_base_q = { 1, AV_TIME_BASE };
    int64_t dts_time = av_rescale_q(frame->pkt_dts, time_base, time_base_q);
    // qDebug() << dts_time;

    if (startDTS < 0) {
        startDTS = dts_time; // 初始化
        startTime = av_gettime();
        // printf("startDTS= %lld, startTime= %lld", startDTS, startTime);
    } else {
        int64_t nowTime = av_gettime() - startTime;

        if ((dts_time - startDTS) > nowTime)
            av_usleep(dts_time - startDTS - nowTime);
    }
}
// 渲染
void SDLVideoPlayer::videoRefresh(AVFrame* a_frame)
{

    // 这里只支持YUV 4:2:0
    int ret = 0;
    if (a_frame) {

        waitForReachPtsTime(a_frame);
        SDL_Rect n_rect = display_rect_.load(std::memory_order_acquire);
        rect_.x = 0;
        rect_.y = 0;
        rect_.w = video_width_;
        rect_.h = video_height_;
        // if (texture_ == nullptr)
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width_, video_height_);
        if (texture_ == nullptr)
            qDebug() << "texture_ == nullptr";
        if ((AVPixelFormat)(a_frame->format) == AVPixelFormat::AV_PIX_FMT_RGB24) {
            ret = SDL_UpdateTexture(texture_, &rect_, a_frame->data[0], a_frame->linesize[0]);
        } else if ((AVPixelFormat)(a_frame->format) == AVPixelFormat::AV_PIX_FMT_YUV420P) {
            ret = SDL_UpdateYUVTexture(texture_, &rect_, a_frame->data[0], a_frame->linesize[0],
                a_frame->data[1], a_frame->linesize[1],
                a_frame->data[2], a_frame->linesize[2]);
        }
        if (ret != 0)
            qDebug() << "SDL_Update*Texture error: " << SDL_GetError();
        ret = SDL_RenderClear(renderer_);
        if (ret != 0)
            qDebug() << "SDL_RenderClear error: " << SDL_GetError();

        if (n_rect.w == 0 && n_rect.h == 0)
            display_rect_ = rect_;
        ret = SDL_RenderCopy(renderer_, texture_, nullptr, nullptr); //&n_rect);
        if (ret != 0)
            qDebug() << "SDL_RenderCopy error: " << SDL_GetError();
        SDL_RenderPresent(renderer_);
        qDebug() << "videoRefresh finished...";
    } else {
    }
    // qDebug() << "no a_frame";
    // qDebug() << "new AVFrame";
}
