#pragma once
#include <string>
#include "../src/orderbook.hpp"

class Bot {
public:
    std::string name;
    double cash        = 100000.0;
    int    position    = 0;
    double realizedPnl = 0.0;

    Bot(std::string n) : name(n) {}

    virtual void onPriceUpdate(double price, LimitOrderBook& lob, int timestep) = 0;

    void recordTrade(double price, int qty, bool buyer) {
        if (buyer) {
            cash        -= price * qty;
            position    += qty;
        } else {
            realizedPnl += (price - avgCostBasis) * qty;  // lock in profit/loss
            cash        += price * qty;
            position    -= qty;
        }
    }

    double pnl(double currentPrice) const {
        return realizedPnl + (cash - 100000.0) + position * currentPrice;
    }

private:
    double avgCostBasis = 0.0;  // tracks average buy price for realized P&L calc
};