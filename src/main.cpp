#include <iostream>
#include "orderbook.hpp"
#include "matching_engine.hpp"
#include "market_data.hpp"

int main() {

    MarketData data;
    data.loadCSV("data/eth_1m.csv");

    LimitOrderBook lob;

    int timestep = 0;

    while(data.hasNext()) {

        auto tick = data.next();
        timestep++;

        std::cout << "\n===== Timestep " << timestep << " =====\n";
        std::cout << "Market Price: " << tick.price << "\n";

        // ----- Bots will go here later -----

        // Example dummy orders for now
        if(timestep % 5 == 0) {
            lob.addOrder({timestep, "RandomBot", tick.price - 10, 1}, true); // buy
        }

        if(timestep % 7 == 0) {
            lob.addOrder({timestep+1000, "RandomBot", tick.price + 10, 1}, false); // sell
        }

        // ----- Matching Engine -----
        matchOrders(lob);

        // Print small orderbook snapshot
        std::cout << "Top Bid: ";
        if(!lob.bids.empty())
            std::cout << lob.bids.begin()->first << "\n";
        else
            std::cout << "None\n";

        std::cout << "Top Ask: ";
        if(!lob.asks.empty())
            std::cout << lob.asks.begin()->first << "\n";
        else
            std::cout << "None\n";

        if(timestep > 100) break; // stop early for testing
    }

    return 0;
}