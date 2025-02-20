#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>
class Thread : public QObject {

public:
    Thread() { }
    ~Thread()
    {
        if (thread_)
            Thread::Stop();
    }
    virtual int Start() = 0;
    virtual void Run() = 0;
    int Stop()
    {

        abort_ = 1; // 保证完成一帧的解包/解码后再退出
        if (thread_) {
            thread_->join(); // 等待thread_退出
            delete thread_;
            thread_ = nullptr;
        }
        return 0;
    }

protected:
    int abort_ = 0;
    std::thread* thread_ = NULL;
};
;
#endif // THREAD_H
