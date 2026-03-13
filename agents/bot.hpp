#pragma once
#include <string>
#include "orderbook.hpp"

class Bot {
public:
    std::string name;

    Bot(std::string n) : name(n) {}

    virtual void onPriceUpdate(double price, LimitOrderBook& lob, int timestep) = 0;
};