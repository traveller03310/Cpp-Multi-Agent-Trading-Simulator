#include "matching_engine.hpp"
#include <algorithm>

static Bot* findBot(std::vector<std::unique_ptr<Bot>> &bots, const std::string &name) {
    for (auto &b : bots)
        if (b->name == name) return b.get();
    return nullptr;
}

void matchOrders(LimitOrderBook &lob,
                 std::vector<std::unique_ptr<Bot>> &bots,
                 std::string &tradeBuffer,
                 int timestep) {

    while (!lob.bids.empty() && !lob.asks.empty()) {
        auto bestBidIt = lob.bids.begin();
        auto bestAskIt = lob.asks.begin();

        while (!bestBidIt->second.empty() &&
               bestBidIt->second.front().quantity == 0)
            bestBidIt->second.pop_front();
        if (bestBidIt->second.empty()) { lob.bids.erase(bestBidIt); continue; }

        while (!bestAskIt->second.empty() &&
               bestAskIt->second.front().quantity == 0)
            bestAskIt->second.pop_front();
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

            if (Bot *buyer  = findBot(bots, buyOrder.agent))
                buyer->recordTrade(tradePrice, executedQty, true);
            if (Bot *seller = findBot(bots, sellOrder.agent))
                seller->recordTrade(tradePrice, executedQty, false);

            buyOrder.quantity  -= executedQty;
            sellOrder.quantity -= executedQty;

            if (buyOrder.quantity == 0) {
                lob.orderIndex.erase(buyOrder.id);
                bestBidIt->second.pop_front();
            }
            if (sellOrder.quantity == 0) {
                lob.orderIndex.erase(sellOrder.id);
                bestAskIt->second.pop_front();
            }

            if (bestBidIt->second.empty()) lob.bids.erase(bestBidIt);
            if (bestAskIt->second.empty()) lob.asks.erase(bestAskIt);
        } else {
            break;
        }
    }
}