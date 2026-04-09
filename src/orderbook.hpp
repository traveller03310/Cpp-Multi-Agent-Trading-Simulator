#pragma once
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include "order.hpp"

class LimitOrderBook {
public:
    // price level → queue of orders at that price
    // sorted highest price first (best bid on top)
    std::map<double, std::queue<Order>, std::greater<double>> bids; 

    // Stores all SELL, default → sorted lowest price first (best ask on top)
    std::map<double, std::queue<Order>> asks;

    // Maps orderID → pointer to Order, allows O(1) cancel by ID w/o searching the whole book
    std::unordered_map<int, Order*> orderIndex;

    void addOrder(const Order& order) {
        if (order.side == Side::BUY) {
            bids[order.price].push(order);  // add to bid queue at this price

            orderIndex[order.id] = &bids[order.price].back(); // ⚠️ One Bug to Watch
            // std::queue uses std::deque internally. If the deque reallocates, 
            // all pointers stored in orderIndex become dangling (pointing to freed memory). 
            // This is a subtle but serious bug in high-frequency trading systems. 
            // Consider using std::list instead, whose pointers remain stable after insertions.
        } 
        else {
            asks[order.price].push(order);  // add to ask queue at this price
            orderIndex[order.id] = &asks[order.price].back();
        }
    }

    bool cancelOrder(int id) {
        auto it = orderIndex.find(id);
        if (it == orderIndex.end()) return false;  // order not found
        it->second->quantity = 0;    // mark order as cancelled (soft delete)
        orderIndex.erase(it);        // remove from index
        return true;
    }
};