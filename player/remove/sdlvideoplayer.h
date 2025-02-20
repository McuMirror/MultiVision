#ifndef SDLVIDEOPLAYER_H
#define SDLVIDEOPLAYER_H

#ifdef __cplusplus ///
extern "C" {
#include "SDL.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
}
#endif
#include "avframequeue.h"

class SDLVideoPlayer {
public:
    SDLVideoPlayer();
    ~SDLVideoPlayer();
    int setVideo(AVFrameQueue* frame_queue, AVStream* video_stream);
    SDL_Window* InitWindow();
    SDL_Window* InitWindow(const void* data);
    int InitTexture();
    int Play();
    std::thread* eventThread = nullptr;
    void pollEvents();
    void extracted();
    void simpleRun();
    void setWin(SDL_Window* win)
    {
        win_ = win;
    }

private:
    void RefreshLoopWaitEvent(SDL_Event* event);
    void videoRefresh(AVFrame*);
    AVFrameQueue* frame_queue_ = nullptr;
    SDL_Event event_;
    SDL_Rect rect_;
    std::atomic<SDL_Rect> display_rect_;
    SDL_Window* win_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    std::atomic<bool> running = true;

    int64_t startDTS = -1;
    int64_t startTime = -1;

    int video_width_ = 0;
    int video_height_ = 0;
    uint8_t* yuv_buf_ = nullptr;
    int yuv_buf_size_ = 0;
    AVStream* video_stream_ = nullptr;

    int64_t time_start_base = -1;
    // SDL_mutex* mutex_;
    void waitForReachPtsTime(AVFrame* frame);

    std::unique_ptr<std::thread> play_thread;
    // std::thread* play_thread = nullptr;
};

#endif // SDLVIDEOPLAYER_H
