#pragma once
#include <string>
#include "../src/orderbook.hpp"

class Bot {
public:
    std::string name;
    double cash     = 0.0;
    int    position = 0;

    Bot(std::string n) : name(n) {}

    virtual void onPriceUpdate(double price, LimitOrderBook& lob, int timestep) = 0;

    void recordTrade(double price, int qty, bool buyer) {
        if (buyer) {
            cash     -= price * qty;
            position += qty;
        } else {
            cash     += price * qty;
            position -= qty;
        }
    }

    double pnl(double currentPrice) const {
        return cash + position * currentPrice;
    }
};