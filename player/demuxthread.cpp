#include "demuxthread.h"
#include "qdebug.h"
#include <QIODevice>

#include <thread>
DemuxThread::DemuxThread(AVPacketQueue* audio_queue, AVPacketQueue* video_queue, QObject* parent)
    : audio_queue_(audio_queue)
    , video_queue_(video_queue)
{
    qDebug() << "DemuxThread";
}

DemuxThread::~DemuxThread()
{
    qDebug() << "~DemuxThread";
    if (thread_)
        Stop();
}

int DemuxThread::Init(const char* url, AVStream** audio_stream, AVStream** video_stream, AVFormatContext** ifmt_ctx)
{

    int ret = 0;
    url_ = url;

    ifmt_ctx_ = avformat_alloc_context(); // 关键, 开辟分配一个内存结构，
    qDebug() << "sizeof ifmt_ctx_ = " << sizeof(AVFormatContext);
    qDebug() << "sizeof AVStream  = " << sizeof(AVStream);
    ret = avformat_open_input(&ifmt_ctx_, url_.c_str(), nullptr, nullptr);
    av_strerror(ret, err2str, sizeof(err2str));

    ret = avformat_find_stream_info(ifmt_ctx_, nullptr);
    av_strerror(ret, err2str, sizeof(err2str));

    audio_index_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    video_index_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    av_dump_format(ifmt_ctx_, 0, url_.c_str(), 0);

    // audio_index = audio_index_;
    // video_index = video_index_;

    *video_stream = ifmt_ctx_->streams[video_index_];
    *audio_stream = ifmt_ctx_->streams[audio_index_];

    *ifmt_ctx = ifmt_ctx_;
    return 0;
}

int DemuxThread::Start()
{
    thread_ = new std::thread(&DemuxThread::Run, this); // 创建一个新的子进程去运行Run
    if (!thread_)
        ; // 防止有问题出现

    return 0;
}

int DemuxThread::Stop()
{
    Thread::Stop();
    avformat_close_input(&ifmt_ctx_);
    return 0;
}
void DemuxThread::Run()
{
    QThread* currentThread = QThread::currentThread();
    qDebug() << "DemuxThread::Run()::thread_id()" << currentThread;

    int ret = 0;
    AVPacket pkt;

    while (abort_ != true) {
        // qDebug() << " DemuxThread::Run()";
        while (audio_queue_->Size() > 100 || video_queue_->Size() > 100) {

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        ret = av_read_frame(ifmt_ctx_, &pkt); // 关键
        if (ret < 0) {

            audio_queue_->MarkEOF();
            video_queue_->MarkEOF();
            // av_strerror();
            abort_ = 1;
            break;
        }
        if (pkt.stream_index == audio_index_) {
            audio_queue_->Push(&pkt);

            // qDebug() << ("audio pkt") << audio_queue_->Size();
        } else if (pkt.stream_index == video_index_) {
            video_queue_->Push(&pkt);
            // qDebug() << ("video pkt :") << video_queue_->Size();
        } else
            av_packet_unref(&pkt);
    }

    qDebug() << "DemuxThread::Run finish";
}

AVCodecParameters* DemuxThread::AudioCodecParameters()
{
    if (audio_index_ != -1) {
        return ifmt_ctx_->streams[audio_index_]->codecpar;
    } else
        return nullptr;
}
AVCodecParameters* DemuxThread::VideoCodecParameters()
{
    if (video_index_ != -1) {

        return ifmt_ctx_->streams[video_index_]->codecpar;
    } else
        return nullptr;
}
// 微秒us
int DemuxThread::get_duration_time()
{
    if (!ifmt_ctx_)
        return 0;
    return ifmt_ctx_->duration;
}

void DemuxThread::on_audioOutput(QIODevice* audioOuput, uint8_t* data, int data_size)
{
    // std::lock_guard<std::mutex> lock(m_mutex);

    // 会在主线程运行
    // QThread* currentThread = QThread::currentThread();
    // qDebug() << "audioOuput->write::thread_id()" << currentThread;
    // Copy the data to a new memory block

    // member: uint8_t* data_;
    //    delete[] data_;
    //    int len = std::strlen(reinterpret_cast<const char*>(data));
    //    data_ = new uint8_t[len];
    //    std::memcpy(data_, data, len);

    auto start = std::chrono::high_resolution_clock::now();
    // reinterpret_cast<const char*>(data)
    audioOuput->write(reinterpret_cast<const char*>(data), data_size); // 在这里写IO进行播放

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // qDebug() << "Function executed in: " << duration.count() << " microseconds.";
}

void DemuxThread::on_seek_frame(int ms)
{
    // 将毫秒转换为秒
    double seconds = ms / 1000.0;

    // 使用 AVStream 的 time_base 将秒数转换为该流的时间戳
    int64_t target_timestamp = av_rescale_q((int64_t)(seconds * AV_TIME_BASE), AV_TIME_BASE_Q, ifmt_ctx_->streams[video_index_]->time_base);

    // 定位到目标时间点
    if (av_seek_frame(ifmt_ctx_, video_index_, target_timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        qDebug() << "Error seeking to frame";
        return;
    }

    // 清空音频和视频队列
    audio_queue_->Clear();
    video_queue_->Clear();

    // 重置解码器缓存，确保从新的位置开始解码
    // 让解码线程做
    // avcodec_flush_buffers(ifmt_ctx_->streams[video_index_]->codec);
}
