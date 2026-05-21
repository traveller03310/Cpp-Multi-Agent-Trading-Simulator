#pragma once
#include <vector>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include "order.hpp"

struct PriceLevel {
    double            price;
    std::deque<Order> orders;
};

class FlatOrderBook {
public:
    // bids: ascending, back() = best bid (highest price)
    std::vector<PriceLevel> bids;
    // asks: ascending, front() = best ask (lowest price)
    std::vector<PriceLevel> asks;

    std::unordered_map<int, Order*> orderIndex;

    void addOrder(const Order& order) {
        if (order.side == Side::BUY) {
            auto& level = findOrInsertBid(order.price);
            level.orders.push_back(order);
            orderIndex[order.id] = &level.orders.back();
        } else {
            auto& level = findOrInsertAsk(order.price);
            level.orders.push_back(order);
            orderIndex[order.id] = &level.orders.back();
        }
    }

    bool cancelOrder(int id) {
        auto it = orderIndex.find(id);
        if (it == orderIndex.end()) return false;
        it->second->quantity = 0;
        orderIndex.erase(it);
        return true;
    }

    bool bidsEmpty() const { return bids.empty(); }
    bool asksEmpty() const { return asks.empty(); }

    PriceLevel& bestBid() { return bids.back(); }
    PriceLevel& bestAsk() { return asks.front(); }

    void removeBestBid() { bids.pop_back(); }
    void removeBestAsk() { asks.erase(asks.begin()); }

private:
    PriceLevel& findOrInsertBid(double price) {
        auto it = std::lower_bound(bids.begin(), bids.end(), price,
            [](const PriceLevel& lvl, double p) { return lvl.price < p; });
        if (it != bids.end() && it->price == price) return *it;
        return *bids.insert(it, PriceLevel{price, {}});
    }

    PriceLevel& findOrInsertAsk(double price) {
        auto it = std::lower_bound(asks.begin(), asks.end(), price,
            [](const PriceLevel& lvl, double p) { return lvl.price < p; });
        if (it != asks.end() && it->price == price) return *it;
        return *asks.insert(it, PriceLevel{price, {}});
    }
};