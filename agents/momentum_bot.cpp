#include "momentum_bot.hpp"
#include "../src/orderbook.hpp"

MomentumBot::MomentumBot(std::string name, int window)
    : Bot(name), window(window) {}

void MomentumBot::onPriceUpdate(double price, LimitOrderBook& lob, int) {
    prices.push_back(price);
    if ((int)prices.size() > window)
        prices.pop_front();

    if ((int)prices.size() < window) return;

    if (shouldBuy())
        lob.addOrder(Order::makeLimitOrder(name, price - 1.0, 1, Side::BUY));
    else if (shouldSell())
        lob.addOrder(Order::makeLimitOrder(name, price + 1.0, 1, Side::SELL));
}

double MomentumBot::recentAvg() const {
    int half = window / 2;
    double sum = 0.0;
    auto it = prices.end();
    for (int i = 0; i < half; ++i) { --it; sum += *it; }
    return sum / half;
}

double MomentumBot::earlierAvg() const {
    int half = window / 2;
    double sum = 0.0;
    auto it = prices.begin();
    for (int i = 0; i < half; ++i, ++it) sum += *it;
    return sum / half;
}

bool MomentumBot::shouldBuy() const {
    return recentAvg() > earlierAvg();
}

bool MomentumBot::shouldSell() const {
    return recentAvg() < earlierAvg();
}