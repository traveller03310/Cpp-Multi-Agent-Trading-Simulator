#pragma once
#include <vector>
#include <string>

struct MarketTick {
    long long timestamp;
    double price;
};

class MarketData {
public:
    std::vector<MarketTick> ticks;
    int index = 0;

    bool loadCSV(const std::string& filename);
    MarketTick next();
    bool hasNext();
};