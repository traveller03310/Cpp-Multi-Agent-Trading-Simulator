#pragma once
#include <cmath>

// Four real cost components missing from most simulators.
// Strategies that look profitable before costs and unprofitable after = useless.

struct TransactionCostModel {
    double makerFeeBps      = 10.0;    // 0.10% — Binance standard maker
    double takerFeeBps      = 10.0;    // 0.10% — Binance standard taker
    double marketImpactBps  = 5.0;     // square-root impact model
    double avgDailyVolume   = 50000.0; // ETH/USDT units
    double borrowRateAnnual = 0.10;    // 10% APR for short positions
    double ticksPerYear     = 525600.0;

    struct CostBreakdown {
        double exchangeFee  = 0.0;
        double spreadCost   = 0.0;
        double marketImpact = 0.0;
        double borrowCost   = 0.0;
        double total() const { return exchangeFee + spreadCost + marketImpact + borrowCost; }
    };

    CostBreakdown compute(double price, int qty, bool isTaker,
                          int position, double spread) const {
        CostBreakdown c;
        double notional = price * qty;

        c.exchangeFee  = notional * ((isTaker ? takerFeeBps : makerFeeBps) / 10000.0);
        c.spreadCost   = (spread / 2.0) * qty;
        c.marketImpact = notional * (marketImpactBps / 10000.0)
                         * std::sqrt(static_cast<double>(qty) / avgDailyVolume);

        if (position < 0)
            c.borrowCost = price * std::abs(position) * (borrowRateAnnual / ticksPerYear);

        return c;
    }
};

inline TransactionCostModel& defaultCostModel() {
    static TransactionCostModel m;
    return m;
}