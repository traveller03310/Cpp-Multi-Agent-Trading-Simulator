#include "market_data.hpp"
#include <fstream>
#include <sstream>

bool MarketData::loadCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        MarketTick tick;

        getline(ss, token, ',');
        tick.timestamp = stoll(token);

        for (int i = 0; i < 3; i++) getline(ss, token, ',');

        getline(ss, token, ',');
        tick.price = stod(token);

        ticks.push_back(tick);
    }

    return true;
}

bool MarketData::hasNext() {
    return index < ticks.size();
}

MarketTick MarketData::next() {
    return ticks[index++];
}