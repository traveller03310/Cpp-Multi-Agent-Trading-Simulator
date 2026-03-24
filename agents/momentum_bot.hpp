#pragma once
#include <deque>
#include <string>
#include "../src/order.hpp"
#include "bot.hpp"

class MomentumBot : public Bot {
private:
    std::deque<double> prices;
    int window;

public:
    MomentumBot(std::string name, int window = 5);
    void onPriceUpdate(double price, LimitOrderBook& lob, int timestep) override;
    bool shouldBuy();
    bool shouldSell();
    Order createOrder(int id);
};