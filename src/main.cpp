#include <iostream>
#include <fstream>
#include "orderbook.hpp"
#include "matching_engine.hpp"
#include "market_data.hpp"
#include "../agents/random_bot.hpp"

int main() {

    MarketData data;
    data.loadCSV("data/eth_1m.csv");

    LimitOrderBook lob;

    RandomBot bot1("BotA");
    RandomBot bot2("BotB");
    RandomBot bot3("BotC");

    // Open trade log CSV
    std::ofstream logFile("data/trade_log.csv");
    logFile << "timestep,buyer,seller,price,quantity\n";

    // Open price log CSV
    std::ofstream priceLog("data/price_log.csv");
    priceLog << "timestep,price\n";

    int timestep = 0;

    while(data.hasNext()) {
        auto tick = data.next();
        timestep++;

        std::cout << "\n=== Timestep " << timestep << " ===\n";
        std::cout << "Price: " << tick.price << "\n";

        // Log price
        priceLog << timestep << "," << tick.price << "\n";

        bot1.onPriceUpdate(tick.price, lob, timestep);
        bot2.onPriceUpdate(tick.price, lob, timestep);
        bot3.onPriceUpdate(tick.price, lob, timestep);

        matchOrders(lob, logFile, timestep);

        if(timestep > 500) break;  // increased to 500 for better metrics
    }

    logFile.close();
    priceLog.close();

    std::cout << "\nLogs saved to data/trade_log.csv and data/price_log.csv\n";
    return 0;
}