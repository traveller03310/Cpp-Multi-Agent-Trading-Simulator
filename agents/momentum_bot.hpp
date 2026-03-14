#pragma once
#include <vector>
#include <string>
#include "../src/order.hpp"

class MomentumBot {
private:
    std::string name;
    std::vector<double> prices;
    int window;

public:
    MomentumBot(std::string name, int window = 5);
    void onPriceUpdate(double price);
    bool shouldBuy();
    bool shouldSell();
    Order createOrder(int id);
};