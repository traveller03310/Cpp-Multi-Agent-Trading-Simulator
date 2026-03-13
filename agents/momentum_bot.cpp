#include "momentum_bot.hpp"

MomentumBot::MomentumBot(std::string name, int window) {
    this->name = name;
    this->window = window;
}

void MomentumBot::onPriceUpdate(double price) {
    prices.push_back(price);

    if (prices.size() > window)
        prices.erase(prices.begin());
}

bool MomentumBot::shouldBuy() {
    if (prices.size() < window) return false;

    return prices.back() > prices.front();
}

bool MomentumBot::shouldSell() {
    if (prices.size() < window) return false;

    return prices.back() < prices.front();
}

Order MomentumBot::createOrder(int id) {

    double price = prices.back();
    int quantity = 1;

    return {id, name, price, quantity};
}