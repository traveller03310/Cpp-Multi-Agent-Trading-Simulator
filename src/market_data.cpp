// Built a CSV parser + time iterator for market simulation

#include "market_data.hpp"
#include <fstream>
#include <sstream>

bool MarketData::loadCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;

    while (getline(file, line)) {   // Keep reading until file ends
        // Reads one full row from CSV at a time until next line
        std::stringstream ss(line);  // Turns string into something you can parse like input
        std::string token;

        MarketTick tick;

        // Reads first column, converts string → long long
        getline(ss, token, ',');   // stops at ',' instead of next line
        tick.timestamp = stoll(token);

        for (int i = 0; i < 3; i++) getline(ss, token, ',');  // skips open, high, low

        // Reads close price, converts to double
        getline(ss, token, ',');
        tick.price = stod(token);

        ticks.push_back(tick);  // Each row becomes one MarketTick
    }

    return true;
}

bool MarketData::hasNext() {
    return index < ticks.size();  // index is int, .size() gives unsigned, thats why used size_t
}

MarketTick MarketData::next() {
    return ticks[index++];
}