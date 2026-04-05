#pragma once
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include "order.hpp"

class LimitOrderBook {
public:
    std::map<double, std::queue<Order>, std::greater<double>> bids;
    std::map<double, std::queue<Order>>                       asks;
    std::unordered_map<int, Order*> orderIndex;

    void addOrder(const Order& order) {
        if (order.side == Side::BUY) {
            bids[order.price].push(order);
            orderIndex[order.id] = &bids[order.price].back();
        } else {
            asks[order.price].push(order);
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