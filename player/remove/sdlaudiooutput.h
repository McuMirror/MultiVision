#ifndef SDLAUDIOOUTPUT_H
#define SDLAUDIOOUTPUT_H
#include "avframequeue.h"
#include <QObject>
#ifdef __cplusplus ///
extern "C" {
//#include "SDL.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
}
#endif

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
} AudioParams;

class SDLAudioOutput {

public:
    SDLAudioOutput(AVFrameQueue* frame_queue, const AudioParams& audio_params);
    ~SDLAudioOutput();
    int Init();
    int DeInit();

public:
    AudioParams src_tgt_; // 解码后的参数
    AudioParams dst_tgt_; // SDL实际输出的格式
    AVFrameQueue* frame_queue_ = nullptr;

    struct SwrContext* swr_ctx_ = nullptr;
    uint8_t* audio_buf_ = nullptr;
    uint8_t* audio_buf1_ = nullptr;
    uint32_t audio_buf_size = 0;
    uint32_t audio_buf1_size = 0;
    uint32_t audio_buf_index = 0;
};

#endif // SDLAUDIOOUTPUT_H
