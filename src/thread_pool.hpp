#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
        : stop_(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    std::future<void> submit(F&& task) {
        auto pt = std::make_shared<std::packaged_task<void()>>(
            std::forward<F>(task));
        std::future<void> fut = pt->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push([pt]{ (*pt)(); });
        }
        cv_.notify_one();
        return fut;
    }

    size_t threadCount() const { return workers_.size(); }

    ~ThreadPool() {
        { std::lock_guard<std::mutex> lock(mutex_); stop_ = true; }
        cv_.notify_all();
        for (auto& t : workers_) t.join();
    }

private:
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex                        mutex_;
    std::condition_variable           cv_;
    bool                              stop_;
};