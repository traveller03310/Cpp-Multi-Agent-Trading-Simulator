#pragma once
#include <deque>
#include <string>
#include "../src/order.hpp"
#include "../src/orderbook.hpp"
#include "../src/flat_orderbook.hpp"
#include "bot.hpp"

class MomentumBot : public Bot {
    std::deque<double> prices;
    int window;

public:
    MomentumBot(std::string name, int window = 5);

    void onPriceUpdate(double price, LimitOrderBook& lob, int timestep) override;
    void onPriceUpdate(double price, FlatOrderBook&  lob, int timestep) override;

private:
    double recentAvg()  const;
    double earlierAvg() const;
    bool shouldBuy()    const;
    bool shouldSell()   const;

    template<typename Book>
    void placeOrders(double price, Book& lob) {
        prices.push_back(price);
        if ((int)prices.size() > window) prices.pop_front();
        if ((int)prices.size() < window) return;

        if (shouldBuy())
            lob.addOrder(Order::makeLimitOrder(name, price - 1.0, 1, Side::BUY));
        else if (shouldSell())
            lob.addOrder(Order::makeLimitOrder(name, price + 1.0, 1, Side::SELL));
    }
};