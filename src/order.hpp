#pragma once
#include <atomic>
#include <limits>
#include <string>

enum class Side {
    BUY,
    SELL
};

struct Order {
    int         id;
    std::string agent;
    double      price;
    int         quantity;
    Side        side;

    static std::atomic<int> nextId;

    static Order makeLimitOrder(std::string agent, double price,
                                int quantity, Side side) {
        return { nextId++, std::move(agent), price, quantity, side };
    }

    static Order makeMarketOrder(std::string agent, int quantity, Side side) {
        double p = (side == Side::BUY)
                       ? std::numeric_limits<double>::max()
                       : 0.0;
        return { nextId++, std::move(agent), p, quantity, side };
    }
};

inline std::atomic<int> Order::nextId{0};