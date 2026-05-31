#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "spsc_queue.hpp"
#include "async_logger.hpp"
#include "market_data.hpp"
#include "orderbook.hpp"
#include "order.hpp"
#include "../agents/bot.hpp"
#include "thread_pool.hpp"

// 4-thread pipeline:
//   Thread 1 (feed)    →[SPSC TickQueue]→
//   Thread 2 (bots)    →[SPSC OrderQueue]→
//   Thread 3 (matcher) →[AsyncLogger]→ CSV
//   Thread 4 (logger)  → disk
//
// Zero mutex in hot path (Threads 1-3)
// False sharing eliminated via alignas(64) per-thread stats

class ThreadedSim {
public:
    // Each stat on its own 64-byte cache line — no false sharing
    struct alignas(64) ThreadStats {
        std::atomic<uint64_t> count{0};
    };

    ThreadedSim(const std::string& instrument,
                const std::string& csvPath,
                std::vector<std::unique_ptr<Bot>>& bots,
                int limit = 500)
        : instrument_(instrument)
        , csvPath_(csvPath)
        , bots_(bots)
        , limit_(limit)
        , done_(false)
        , botDone_(false) {}

    long long run() {
        auto wallStart = std::chrono::high_resolution_clock::now();

        AsyncLogger logger("data/trade_log_" + instrument_ + "_threaded.csv");
        logger.writeHeader("timestep,buyer,seller,price,quantity\n");

        std::thread t1([this]          { feedThread(); });
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        std::thread t2([this]          { botThread(); });
        std::thread t3([this, &logger] { matchThread(logger); });

        t1.join();
        t2.join();
        t3.join();
        logger.stop();

        auto wallEnd = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   wallEnd - wallStart).count();
    }

    uint64_t ticksProcessed()  const { return stats_[0].count.load(); }
    uint64_t ordersPlaced()    const { return stats_[1].count.load(); }
    uint64_t tradesExecuted()  const { return stats_[2].count.load(); }
    size_t   poolThreads()     const { return pool_.threadCount(); }

private:
    // Thread 1: reads CSV, pushes ticks to tickQueue_
    void feedThread() {
        MarketData data;
        if (!data.loadCSV(csvPath_)) {
            done_.store(true, std::memory_order_release);
            return;
        }
        int count = 0;
        while (data.hasNext() && count < limit_) {
            while (!tickQueue_.push(data.next()))
                std::this_thread::yield();
            stats_[0].count.fetch_add(1, std::memory_order_relaxed);
            count++;
        }
        done_.store(true, std::memory_order_release);
    }

    // Thread 2: runs bots against a staging LOB, flushes new orders to orderQueue_
    // Bots use normal virtual dispatch → LimitOrderBook, no bot code changes needed
    void botThread() {
        LimitOrderBook stagingLob;
        int lastOrderId = Order::nextId.load(std::memory_order_relaxed);

        while (true) {
            if (auto tick = tickQueue_.pop()) {
                lastPrice_ = tick->price;
                for (auto& bot : bots_)
                    bot->onPriceUpdate(tick->price, stagingLob, 0);
                flushNewOrders(stagingLob, lastOrderId);
            } else {
                if (done_.load(std::memory_order_acquire)) break;
                std::this_thread::yield();
            }
        }
        botDone_.store(true, std::memory_order_release);
    }

    // Scans staging LOB for new orders, pushes them to SPSC queue, clears staging LOB
    void flushNewOrders(LimitOrderBook& stagingLob, int& lastOrderId) {
        int currentMax = Order::nextId.load(std::memory_order_relaxed);
        if (currentMax == lastOrderId) return;

        for (auto& [price, orderList] : stagingLob.bids)
            for (auto& order : orderList)
                if (order.id > lastOrderId && order.quantity > 0) {
                    while (!orderQueue_.push(order)) std::this_thread::yield();
                    stats_[1].count.fetch_add(1, std::memory_order_relaxed);
                }

        for (auto& [price, orderList] : stagingLob.asks)
            for (auto& order : orderList)
                if (order.id > lastOrderId && order.quantity > 0) {
                    while (!orderQueue_.push(order)) std::this_thread::yield();
                    stats_[1].count.fetch_add(1, std::memory_order_relaxed);
                }

        stagingLob.bids.clear();
        stagingLob.asks.clear();
        stagingLob.orderIndex.clear();
        lastOrderId = currentMax;
    }

    // Thread 3: inserts orders into real LOB, matches, logs via AsyncLogger
    void matchThread(AsyncLogger& logger) {
        LimitOrderBook lob;
        int timestep = 0;

        while (true) {
            if (auto order = orderQueue_.pop()) {
                timestep++;
                lob.addOrder(*order);

                std::string tradeLines;
                matchAndLog(lob, tradeLines, timestep);

                if (!tradeLines.empty()) {
                    logger.log(tradeLines);
                    stats_[2].count.fetch_add(1, std::memory_order_relaxed);
                }
            } else {
                if (botDone_.load(std::memory_order_acquire) && orderQueue_.empty())
                    break;
                std::this_thread::yield();
            }
        }
    }

    void matchAndLog(LimitOrderBook& lob, std::string& out, int timestep) {
        while (!lob.bids.empty() && !lob.asks.empty()) {
            auto bidIt = lob.bids.begin();
            auto askIt = lob.asks.begin();

            while (!bidIt->second.empty() && bidIt->second.front().quantity == 0)
                bidIt->second.pop_front();
            if (bidIt->second.empty()) { lob.bids.erase(bidIt); continue; }

            while (!askIt->second.empty() && askIt->second.front().quantity == 0)
                askIt->second.pop_front();
            if (askIt->second.empty()) { lob.asks.erase(askIt); continue; }

            if (bidIt->first >= askIt->first) {
                Order& buy  = bidIt->second.front();
                Order& sell = askIt->second.front();
                int    qty   = std::min(buy.quantity, sell.quantity);
                double price = sell.price;

                out += std::to_string(timestep) + ','
                     + buy.agent  + ','
                     + sell.agent + ','
                     + std::to_string(price) + ','
                     + std::to_string(qty)   + '\n';

                for (auto& b : bots_) {
                    if (b->name == buy.agent)  b->recordTrade(price, qty, true);
                    if (b->name == sell.agent) b->recordTrade(price, qty, false);
                }

                buy.quantity  -= qty;
                sell.quantity -= qty;
                if (buy.quantity  == 0) { lob.orderIndex.erase(buy.id);  bidIt->second.pop_front(); }
                if (sell.quantity == 0) { lob.orderIndex.erase(sell.id); askIt->second.pop_front(); }
                if (bidIt->second.empty()) lob.bids.erase(bidIt);
                if (askIt->second.empty()) lob.asks.erase(askIt);
            } else {
                break;
            }
        }
    }

    std::string instrument_;
    std::string csvPath_;
    std::vector<std::unique_ptr<Bot>>& bots_;
    int limit_;

    SPSCQueue<MarketTick, 1024> tickQueue_;
    SPSCQueue<Order, 4096>      orderQueue_;

    std::atomic<bool> done_{false};
    std::atomic<bool> botDone_{false};
    double            lastPrice_{0.0};

    ThreadStats stats_[3];

    ThreadPool pool_;
};