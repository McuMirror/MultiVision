#ifndef AVPACKETQUEUE_H
#define AVPACKETQUEUE_H

#include "queue.h"
#include <atomic>
#include <condition_variable>
#ifdef __cplusplus ///
extern "C" {
#include "libavcodec/avcodec.h"
}
#endif

class AVPacketQueue {
public:
    AVPacketQueue();
    ~AVPacketQueue();
    void Abort();
    void Release();
    int Size();

    int Push(AVPacket* val);
    void Clear();
    AVPacket* Pop(const int timeout);

    void MarkEOF()
    {
        eof_.store(true);
    }

    bool IsEOF()
    {
        return eof_.load();
    }

private:
    std::atomic<bool> eof_ { false }; // 队列终止标志，true表示结束
    Queue<AVPacket*> queue_; // 组合方式
};

#endif // AVPACKETQUEUE_H
