#include "avpacketqueue.h"

AVPacketQueue::AVPacketQueue()
{
}

AVPacketQueue::~AVPacketQueue()
{
}

void AVPacketQueue::Abort()
{
    queue_.Abort();
}

void AVPacketQueue::Release()
{
    while (true) {
        AVPacket* pkt = nullptr;
        int ret = queue_.Pop(pkt, 1);
        if (ret >= 0) {
            av_packet_free(&pkt);
            continue;
        } else
            break;
    }
}

int AVPacketQueue::Size()
{
    return queue_.Size();
}

int AVPacketQueue::Push(AVPacket* pkt)
{
    AVPacket* tmp_pkt = av_packet_alloc();
    av_packet_move_ref(tmp_pkt, pkt); // 移动语义——转移所有权
    return queue_.Push(tmp_pkt);
}

void AVPacketQueue::Clear()
{
    queue_.Clear();
}

AVPacket* AVPacketQueue::Pop(const int timeout)
{
    AVPacket* tmp_pkt = nullptr;
    int ret = queue_.Pop(tmp_pkt, timeout);
    // 检测ret是否正
    return tmp_pkt;
}
