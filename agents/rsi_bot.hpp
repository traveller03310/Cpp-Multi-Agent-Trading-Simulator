#pragma once
#include <deque>
#include "bot.hpp"
#include "../src/orderbook.hpp"
#include "../src/flat_orderbook.hpp"

class RSIBot : public Bot {
    int    period;
    double oversold;
    double overbought;
    std::deque<double> prices;

public:
    RSIBot(std::string name, int period = 14,
           double oversold = 30.0, double overbought = 70.0)
        : Bot(name), period(period), oversold(oversold), overbought(overbought) {}

    void onPriceUpdate(double price, LimitOrderBook& lob, int) override { placeOrders(price, lob); }
    void onPriceUpdate(double price, FlatOrderBook&  lob, int) override { placeOrders(price, lob); }

private:
    double computeRSI() const {
        double avgGain = 0.0, avgLoss = 0.0;
        for (int i = 1; i <= period; ++i) {
            double change = prices[i] - prices[i - 1];
            if (change > 0) avgGain += change;
            else            avgLoss -= change;
        }
        avgGain /= period;
        avgLoss /= period;
        if (avgLoss == 0.0) return 100.0;
        return 100.0 - (100.0 / (1.0 + avgGain / avgLoss));
    }

    template<typename Book>
    void placeOrders(double price, Book& lob) {
        prices.push_back(price);
        if ((int)prices.size() > period + 1) prices.pop_front();
        if ((int)prices.size() < period + 1) return;

        double rsi = computeRSI();
        if (rsi < oversold)
            lob.addOrder(Order::makeLimitOrder(name, price - 1.0, 1, Side::BUY));
        else if (rsi > overbought)
            lob.addOrder(Order::makeLimitOrder(name, price + 1.0, 1, Side::SELL));
    }
};