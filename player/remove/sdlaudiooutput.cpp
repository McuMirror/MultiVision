#include "sdlaudiooutput.h"

#include <qdebug>
SDLAudioOutput::SDLAudioOutput(AVFrameQueue* frame_queue, const AudioParams& audio_params)
    : src_tgt_(audio_params)
    , frame_queue_(frame_queue)
{
}
FILE* dump_pcm = nullptr;
void fill_audio_pcm(void* udata, Uint8* stream, int len)
{
    // 1.从frame_queue读取解码后的PCM数据，填充到stream
    // 2.len = 4000字节，一个frame有6000字节，一次读取了4000，剩2000
    SDLAudioOutput* is = (SDLAudioOutput*)udata;
    int len1 = 0;
    int audio_size = 0;
    if (!dump_pcm) {
        dump_pcm = fopen("dump_pcm", "wb");
    }
    while (len > 0) {
        if (is->audio_buf_index == is->audio_buf_size) {
            is->audio_buf_index = 0;
            AVFrame* frame = is->frame_queue_->Pop(10);
            if (frame) {
                if (((frame->format != is->dst_tgt_.fmt)
                        || (frame->sample_rate != is->dst_tgt_.freq)
                        || (frame->channel_layout != is->dst_tgt_.channel_layout))
                    && (!is->swr_ctx_)) {
                    is->swr_ctx_ = swr_alloc_set_opts(nullptr,
                        is->dst_tgt_.channel_layout,
                        (enum AVSampleFormat)is->dst_tgt_.fmt,
                        is->dst_tgt_.freq,
                        frame->channel_layout,
                        (enum AVSampleFormat)frame->format,
                        frame->sample_rate,
                        0,
                        nullptr);
                    if (!is->swr_ctx_ || swr_init(is->swr_ctx_) < 0) {
                        qWarning() << "Cannot create sample rate converter";
                        swr_free((SwrContext**)(&is->swr_ctx_));
                        return;
                    }
                }
                if (is->swr_ctx_) {
                    const uint8_t** in = (const uint8_t**)frame->extended_data;
                    uint8_t** out = &is->audio_buf1_;
                    int out_samples = frame->nb_samples * is->dst_tgt_.freq / frame->sample_rate + 256;
                    int out_bytes = av_samples_get_buffer_size(nullptr, is->dst_tgt_.channels, out_samples, is->dst_tgt_.fmt, 0);
                    //                    if (out_bytes < 0) {
                    //                        qWarning() << "av_samples_get_buffer_size failed";
                    //                        return;
                    //                    }
                    av_fast_malloc(&is->audio_buf1_, &is->audio_buf1_size, out_bytes);

                    int len2 = swr_convert(is->swr_ctx_, out, out_samples, in, frame->nb_samples);
                    if (len2 < 0) {
                        qWarning() << "swr_convert failed";
                        return;
                    }
                    is->audio_buf_ = is->audio_buf1_;
                    is->audio_buf_size = av_samples_get_buffer_size(nullptr, is->dst_tgt_.channels, len2, is->dst_tgt_.fmt, 1);
                } else {
                    // 没有重采样
                    audio_size = av_samples_get_buffer_size(nullptr, frame->channels, frame->nb_samples, (enum AVSampleFormat)frame->format, 1);
                    av_fast_malloc(&is->audio_buf1_, &is->audio_buf1_size, audio_size);
                    is->audio_buf_ = is->audio_buf1_;
                    is->audio_buf_size = audio_size;
                    memcpy(is->audio_buf_, frame->data[0], audio_size);
                }
                av_frame_free(&frame);
            } else {
                is->audio_buf_ = nullptr;
                is->audio_buf_index = 512;
            }
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        if (!is->audio_buf_)
            memset(stream, 0, len1);
        else {
            // 真正拷贝有效的数据
            memcpy(stream, is->audio_buf_ + is->audio_buf_index, len1);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}
int SDLAudioOutput::Init()
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        qDebug() << "SDL_Init failed";
        return -1;
    }
    SDL_AudioSpec wanted_spec, spec;
    wanted_spec.channels = src_tgt_.channels;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.callback = fill_audio_pcm;
    wanted_spec.userdata = this;
    wanted_spec.samples = src_tgt_.frame_size; // 采样数量
    int ret = SDL_OpenAudio(&wanted_spec, &spec);
    if (ret < 0) {
        qWarning() << "SDL_OpenAudio failed";
        return -1;
    }
    dst_tgt_.channels = spec.channels;
    dst_tgt_.fmt = AV_SAMPLE_FMT_S16;
    dst_tgt_.freq = spec.freq;

    dst_tgt_.channel_layout = av_get_default_channel_layout(spec.channels);
    SDL_PauseAudio(0);
    qWarning() << "SDLAudioOutput::Init() finished";
    return 0;
}

int SDLAudioOutput::DeInit()
{
    return 0;
}
