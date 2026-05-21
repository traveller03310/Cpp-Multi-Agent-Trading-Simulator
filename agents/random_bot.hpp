#pragma once
#include "bot.hpp"
#include <random>
#include "../src/orderbook.hpp"

class RandomBot : public Bot {
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist{0, 9};

public:
    RandomBot(std::string n)
        : Bot(n), rng(std::random_device{}()) {}

    void onPriceUpdate(double price, LimitOrderBook& lob, int) override {
        int r = dist(rng);
        if (r < 3)
            lob.addOrder(Order::makeLimitOrder(name, price - 5, 1, Side::BUY));
        else if (r > 7)
            lob.addOrder(Order::makeLimitOrder(name, price + 5, 1, Side::SELL));
    }
};