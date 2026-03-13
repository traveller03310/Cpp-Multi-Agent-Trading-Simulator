#pragma once
#include <string>

enum class Side {
    BUY,
    SELL
};

struct Order {
    int id;
    std::string agent;
    double price;
    int quantity;
};