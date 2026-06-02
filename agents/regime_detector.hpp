#pragma once
#include <deque>
#include <vector>
#include <cmath>
#include <string>

enum class Regime { TRENDING, MEAN_REVERT, UNDEFINED };

inline const char* regimeName(Regime r) {
    switch (r) {
        case Regime::TRENDING:    return "TRENDING";
        case Regime::MEAN_REVERT: return "MEAN_REVERT";
        default:                  return "UNDEFINED";
    }
}

class RegimeDetector {
public:
    int    volWindow  = 20;
    int    acfWindow  = 20;
    double acfThresh  = 0.15;
    double volThresh  = 0.002;

    Regime currentRegime = Regime::UNDEFINED;
    double lastVol       = 0.0;
    double lastAcf       = 0.0;

    void update(double price) {
        prices_.push_back(price);
        if ((int)prices_.size() > volWindow + 1) prices_.pop_front();
        if ((int)prices_.size() < volWindow + 1) { currentRegime = Regime::UNDEFINED; return; }

        std::vector<double> rets;
        rets.reserve(prices_.size() - 1);
        for (size_t i = 1; i < prices_.size(); ++i)
            rets.push_back(std::log(prices_[i] / prices_[i - 1]));

        double mean = 0.0;
        for (double r : rets) mean += r;
        mean /= rets.size();

        double var = 0.0;
        for (double r : rets) var += (r - mean) * (r - mean);
        lastVol = std::sqrt(var / rets.size());

        double cov = 0.0;
        for (size_t i = 1; i < rets.size(); ++i)
            cov += (rets[i] - mean) * (rets[i - 1] - mean);
        cov /= (rets.size() - 1);
        lastAcf = (var > 1e-12) ? cov / (var / rets.size()) : 0.0;

        if      (lastAcf >  acfThresh) currentRegime = Regime::TRENDING;
        else if (lastAcf < -acfThresh) currentRegime = Regime::MEAN_REVERT;
        else                           currentRegime = Regime::UNDEFINED;
    }

    bool isTrending()   const { return currentRegime == Regime::TRENDING; }
    bool isMeanRevert() const { return currentRegime == Regime::MEAN_REVERT; }

private:
    std::deque<double> prices_;
};