#pragma once
#include <iostream>
#include <map>
#include <list>
#include <string>
#include <unordered_map>
#include "order.hpp"

class LimitOrderBook {
public:
    // std::list guarantees pointer/iterator stability after insertions —
    // unlike std::queue (backed by std::deque), which can reallocate and
    // invalidate every pointer stored in orderIndex. Using std::list
    // eliminates the dangling-pointer bug entirely.

    // sorted highest price first (best bid on top)
    std::map<double, std::list<Order>, std::greater<double>> bids;

    // sorted lowest price first (best ask on top)
    std::map<double, std::list<Order>> asks;

    // Maps orderID → pointer to Order, allows O(1) cancel by ID
    // Safe now: std::list nodes never move in memory after insertion.
    std::unordered_map<int, Order*> orderIndex;

    void addOrder(const Order& order) {
        if (order.side == Side::BUY) {
            bids[order.price].push_back(order);
            orderIndex[order.id] = &bids[order.price].back();
        } else {
            asks[order.price].push_back(order);
            orderIndex[order.id] = &asks[order.price].back();
        }
    }

    bool cancelOrder(int id) {
        auto it = orderIndex.find(id);
        if (it == orderIndex.end()) return false;
        it->second->quantity = 0;
        orderIndex.erase(it);
        return true;
    }
};