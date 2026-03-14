#include "matching_engine.hpp"
#include <fstream>
#include <algorithm>

void matchOrders(LimitOrderBook &lob, std::ofstream &logFile, int timestep) {
    while(!lob.bids.empty() && !lob.asks.empty()) {
        auto bestBidIt  = lob.bids.begin();
        auto bestAskIt  = lob.asks.begin();

        if(bestBidIt->first >= bestAskIt->first) {
            Order &buyOrder  = bestBidIt->second.front();
            Order &sellOrder = bestAskIt->second.front();

            int    executedQty  = std::min(buyOrder.quantity, sellOrder.quantity);
            double tradePrice   = sellOrder.price;

            std::cout << "Trade executed: " << executedQty
                      << " ETH at " << tradePrice
                      << " between " << buyOrder.agent
                      << " and " << sellOrder.agent << "\n";

            // Log to CSV: timestep, buyer, seller, price, quantity
            logFile << timestep << ","
                    << buyOrder.agent  << ","
                    << sellOrder.agent << ","
                    << tradePrice      << ","
                    << executedQty     << "\n";

            buyOrder.quantity  -= executedQty;
            sellOrder.quantity -= executedQty;

            if(buyOrder.quantity  == 0) bestBidIt->second.pop();
            if(sellOrder.quantity == 0) bestAskIt->second.pop();

            if(bestBidIt->second.empty()) lob.bids.erase(bestBidIt);
            if(bestAskIt->second.empty()) lob.asks.erase(bestAskIt);
        } else {
            break;
        }
    }
}