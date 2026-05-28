#pragma once
#include <string>
#include <thread>
#include <fstream>
#include <atomic>
#include "spsc_queue.hpp"

// Thread 4 — writes to disk without ever blocking the matching engine.
// Matching engine calls log() and returns immediately.
// Logger thread drains the queue and flushes at its own pace.
class AsyncLogger {
public:
    AsyncLogger(const std::string& filename)
        : out_(filename), stop_(false) {
        thread_ = std::thread([this]{ loggerLoop(); });
    }

    // Called by Thread 3 (matching engine) — non-blocking
    void log(const std::string& line) {
        while (!queue_.push(line))
            std::this_thread::yield();
    }

    void writeHeader(const std::string& header) {
        out_ << header;
        out_.flush();
    }

    // Called by main thread when simulation ends
    // Drains remaining queue items then exits
    void stop() {
        stop_.store(true, std::memory_order_release);
        thread_.join();
    }

private:
    void loggerLoop() {
        while (true) {
            bool wrote = false;
            while (auto item = queue_.pop()) {
                out_ << *item;
                wrote = true;
            }
            if (wrote) out_.flush();

            if (stop_.load(std::memory_order_acquire) && queue_.empty())
                break;

            std::this_thread::yield();
        }
        out_.flush();
    }

    SPSCQueue<std::string, 4096> queue_;
    std::ofstream out_;
    std::atomic<bool> stop_;
    std::thread thread_;
};