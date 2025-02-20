#ifndef QUEUE_H
#define QUEUE_H
#include <atomic>
#include <condition_variable>
#include <queue>

template <typename T>
class Queue {
public:
    Queue() { }
    ~Queue() { }
    void Abort()
    {
        abort_ = 1;
        cond_.notify_all(); // notify_all用于唤醒所有在当前对象上等待的线程
    }
    int Push(T val)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (abort_ == 1)
            return -1;
        queue_.push(val);
        cond_.notify_one();
        return 0;
    }
    // 弹出队头的包，并赋给val
    int Pop(T& val, const int timeout = 0)
    {
        // std::unique_lock<std::mutex> lock(mutex_);

        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.empty()) {
            cond_.wait_for(lock, std::chrono::milliseconds(timeout), [this] { return !queue_.empty() | abort_; });
        }
        if (abort_ == 1)
            return -1;
        if (queue_.empty())
            return -2;
        val = queue_.front();
        queue_.pop();
        return 0;
    }
    int Front(T& val)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        //... // 判断abort_，以及队列非空
        if (abort_ == 1)
            return -1;
        if (queue_.empty())
            return -2;

        val = queue_.front();
        return 0;
    }
    int Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

private:
    int abort_ = 0;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<T> queue_;
};

#endif // QUEUE_H
