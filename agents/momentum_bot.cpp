#include "momentum_bot.hpp"

MomentumBot::MomentumBot(std::string name, int window)
    : Bot(name), window(window) {}

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

bool MomentumBot::shouldBuy()  const { return recentAvg() > earlierAvg(); }
bool MomentumBot::shouldSell() const { return recentAvg() < earlierAvg(); }