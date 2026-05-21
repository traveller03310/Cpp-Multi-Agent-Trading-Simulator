#include "flat_matching_engine.hpp"
#include <algorithm>
#include <iostream>

static Bot* findBot(std::vector<std::unique_ptr<Bot>> &bots, const std::string &name) {
    for (auto &b : bots)
        if (b->name == name) return b.get();
    return nullptr;
}

void matchOrdersFlat(FlatOrderBook &lob,
                     std::vector<std::unique_ptr<Bot>> &bots,
                     std::string &tradeBuffer,
                     int timestep) {

    while (!lob.bidsEmpty() && !lob.asksEmpty()) {
        PriceLevel &bidLevel = lob.bestBid();
        PriceLevel &askLevel = lob.bestAsk();

        while (!bidLevel.orders.empty() && bidLevel.orders.front().quantity == 0)
            bidLevel.orders.pop_front();
        if (bidLevel.orders.empty()) { lob.removeBestBid(); continue; }

        while (!askLevel.orders.empty() && askLevel.orders.front().quantity == 0)
            askLevel.orders.pop_front();
        if (askLevel.orders.empty()) { lob.removeBestAsk(); continue; }

        if (bidLevel.price >= askLevel.price) {
            Order &buyOrder  = bidLevel.orders.front();
            Order &sellOrder = askLevel.orders.front();

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
                bidLevel.orders.pop_front();
            }
            if (sellOrder.quantity == 0) {
                lob.orderIndex.erase(sellOrder.id);
                askLevel.orders.pop_front();
            }

            if (bidLevel.orders.empty()) lob.removeBestBid();
            if (askLevel.orders.empty()) lob.removeBestAsk();
        } else {
            break;
        }
    }
}