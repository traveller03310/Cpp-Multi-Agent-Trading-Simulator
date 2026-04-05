#include "momentum_bot.hpp"
#include "../src/orderbook.hpp"

MomentumBot::MomentumBot(std::string name, int window)
    : Bot(name), window(window) {}

void MomentumBot::onPriceUpdate(double price, LimitOrderBook& lob, int) {
    prices.push_back(price);
    if ((int)prices.size() > window)
        prices.pop_front();

    if (shouldBuy())
        lob.addOrder(Order::makeLimitOrder(name, price - 1.0, 1, Side::BUY));
    else if (shouldSell())
        lob.addOrder(Order::makeLimitOrder(name, price + 1.0, 1, Side::SELL));
}

bool MomentumBot::shouldBuy() const {
    if ((int)prices.size() < window) return false;
    return prices.back() > prices.front();
}

bool MomentumBot::shouldSell() const {
    if ((int)prices.size() < window) return false;
    return prices.back() < prices.front();
}