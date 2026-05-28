#pragma once
#include <atomic>
#include <array>
#include <optional>
#include <cstddef>

template<typename T, size_t N>
class SPSCQueue {
public:
    // push — called by PRODUCER only
    // returns false if queue is full
    bool push(const T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next = (tail + 1) % N;

        // acquire: see latest head_ written by consumer
        if (next == head_.load(std::memory_order_acquire))
            return false;  // full

        buffer_[tail] = item;

        // release: "item is written, consumer can now see it"
        tail_.store(next, std::memory_order_release);
        return true;
    }

    // pop — called by CONSUMER only
    // returns nullopt if queue is empty
    std::optional<T> pop() {
        size_t head = head_.load(std::memory_order_relaxed);

        // acquire: see latest tail_ written by producer
        // also guarantees we see the buffer write that happened before it
        if (head == tail_.load(std::memory_order_acquire))
            return std::nullopt;  // empty

        T item = buffer_[head];

        // release: "slot is free, producer can reuse it"
        head_.store((head + 1) % N, std::memory_order_release);
        return item;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) ==
               tail_.load(std::memory_order_relaxed);
    }

    size_t size() const {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        return (tail - head + N) % N;
    }

private:
    // alignas(64) = each atomic on its own 64-byte cache line
    // without this: producer writing tail_ invalidates consumer's cache line
    // even though consumer only cares about head_ — that's false sharing
    alignas(64) std::atomic<size_t> head_{0};  // consumer advances
    alignas(64) std::atomic<size_t> tail_{0};  // producer advances

    std::array<T, N> buffer_;
};