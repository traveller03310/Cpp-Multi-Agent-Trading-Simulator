#include "matching_engine.hpp"
#include <algorithm>

void matchOrders(LimitOrderBook &lob, std::string &tradeBuffer, int timestep) {
    while (!lob.bids.empty() && !lob.asks.empty()) {
        auto bestBidIt = lob.bids.begin();
        auto bestAskIt = lob.asks.begin();

        while (!bestBidIt->second.empty() &&
               bestBidIt->second.front().quantity == 0)
            bestBidIt->second.pop();
        if (bestBidIt->second.empty()) { lob.bids.erase(bestBidIt); continue; }

        while (!bestAskIt->second.empty() &&
               bestAskIt->second.front().quantity == 0)
            bestAskIt->second.pop();
        if (bestAskIt->second.empty()) { lob.asks.erase(bestAskIt); continue; }

        if (bestBidIt->first >= bestAskIt->first) {
            Order &buyOrder  = bestBidIt->second.front();
            Order &sellOrder = bestAskIt->second.front();

            int    executedQty = std::min(buyOrder.quantity, sellOrder.quantity);
            double tradePrice  = sellOrder.price;

            std::cout << "Trade executed: " << executedQty
                      << " ETH at " << tradePrice
                      << " between " << buyOrder.agent
                      << " and "    << sellOrder.agent << '\n';

            tradeBuffer += std::to_string(timestep)    + ','
                         + buyOrder.agent              + ','
                         + sellOrder.agent             + ','
                         + std::to_string(tradePrice)  + ','
                         + std::to_string(executedQty) + '\n';

            buyOrder.quantity  -= executedQty;
            sellOrder.quantity -= executedQty;

            if (buyOrder.quantity == 0) {
                lob.orderIndex.erase(buyOrder.id);
                bestBidIt->second.pop();
            }
            if (sellOrder.quantity == 0) {
                lob.orderIndex.erase(sellOrder.id);
                bestAskIt->second.pop();
            }

            if (bestBidIt->second.empty()) lob.bids.erase(bestBidIt);
            if (bestAskIt->second.empty()) lob.asks.erase(bestAskIt);
        } else {
            break;
        }
    }
}