#pragma once
#include "bot.hpp"
#include <cstdlib>
#include "../src/orderbook.hpp"

class RandomBot : public Bot {
public:
    RandomBot(std::string n) : Bot(n) {}

    void onPriceUpdate(double price, LimitOrderBook& lob, int) override {
        int r = rand() % 10;
        if (r < 3)
            lob.addOrder(Order::makeLimitOrder(name, price - 5, 1, Side::BUY));
        else if (r > 7)
            lob.addOrder(Order::makeLimitOrder(name, price + 5, 1, Side::SELL));
    }
};