#pragma once
#include <string>

class LimitOrderBook;
class FlatOrderBook;

class Bot {
public:
    std::string name;
    double cash        = 100000.0;
    int    position    = 0;
    double realizedPnl = 0.0;

    Bot(std::string n) : name(n) {}
    virtual ~Bot() = default;

    virtual void onPriceUpdate(double price, LimitOrderBook& lob, int timestep);
    virtual void onPriceUpdate(double price, FlatOrderBook&  lob, int timestep);

    void recordTrade(double price, int qty, bool buyer) {
        if (buyer) {
            cash     -= price * qty;
            position += qty;
        } else {
            realizedPnl += (price - avgCostBasis) * qty;
            cash        += price * qty;
            position    -= qty;
        }
    }

    double pnl(double currentPrice) const {
        return realizedPnl + (cash - 100000.0) + position * currentPrice;
    }

private:
    double avgCostBasis = 0.0;
};