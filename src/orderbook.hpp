#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include "order.hpp"
#include "ring_buffer.hpp"

class LimitOrderBook {
public:
    static constexpr size_t LEVEL_CAPACITY = 64;
    using OrderQueue = RingBuffer<Order, LEVEL_CAPACITY>;

    std::map<double, OrderQueue, std::greater<double>> bids;
    std::map<double, OrderQueue>                       asks;
    std::unordered_map<int, Order*>                    orderIndex;

    // Volume filled at each price level — used for queue position gating
    std::unordered_map<double, int> bidVolumeFilled;
    std::unordered_map<double, int> askVolumeFilled;

    void addOrder(const Order& order) {
        if (order.side == Side::BUY) {
            auto& q = bids[order.price];
            Order o = order;
            o.queuePos = queueDepth(q);
            q.push_back(o);
            orderIndex[o.id] = q.back_ptr();
        } else {
            auto& q = asks[order.price];
            Order o = order;
            o.queuePos = queueDepth(q);
            q.push_back(o);
            orderIndex[o.id] = q.back_ptr();
        }
    }

    bool cancelOrder(int id) {
        auto it = orderIndex.find(id);
        if (it == orderIndex.end()) return false;
        it->second->quantity = 0;
        orderIndex.erase(it);
        return true;
    }

    // imbalance ∈ [-1,+1]: +1 = pure bid pressure, -1 = pure ask pressure
    double imbalance(int levels = 5) const {
        double bidVol = 0.0, askVol = 0.0;
        int n = 0;
        for (auto& [price, q] : bids) { bidVol += queueDepth(q); if (++n >= levels) break; }
        n = 0;
        for (auto& [price, q] : asks) { askVol += queueDepth(q); if (++n >= levels) break; }
        double total = bidVol + askVol;
        return total == 0.0 ? 0.0 : (bidVol - askVol) / total;
    }

    double bestBid()  const { return bids.empty() ? 0.0 : bids.begin()->first; }
    double bestAsk()  const { return asks.empty() ? 0.0 : asks.begin()->first; }
    double midPrice() const { return (bestBid() + bestAsk()) / 2.0; }
    double spread()   const { return bestAsk() - bestBid(); }

private:
    static int queueDepth(const OrderQueue& q) {
        int total = 0;
        for (const auto& o : q) total += o.quantity;
        return total;
    }
};