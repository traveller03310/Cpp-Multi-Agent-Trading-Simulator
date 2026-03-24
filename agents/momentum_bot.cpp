#include "momentum_bot.hpp"

MomentumBot::MomentumBot(std::string name, int window)
    : Bot(name), window(window) {}

void MomentumBot::onPriceUpdate(double price, LimitOrderBook& lob, int timestep) {
    prices.push_back(price);
    if ((int)prices.size() > window)
        prices.pop_front();
}

bool MomentumBot::shouldBuy() {
    if ((int)prices.size() < window) return false;
    return prices.back() > prices.front();
}

bool MomentumBot::shouldSell() {
    if ((int)prices.size() < window) return false;
    return prices.back() < prices.front();
}

Order MomentumBot::createOrder(int id) {
    double price = prices.back();
    int quantity = 1;
    return {id, name, price, quantity};
}