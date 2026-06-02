#include "matching_engine.hpp"
#include "rdtsc.hpp"
#include <algorithm>
#include <cstring>

static Bot* findBot(std::vector<std::unique_ptr<Bot>>& bots, const char* name) {
    for (auto& b : bots)
        if (std::strcmp(b->name.c_str(), name) == 0) return b.get();
    return nullptr;
}

void matchOrders(LimitOrderBook& lob,
                 std::vector<std::unique_ptr<Bot>>& bots,
                 std::string& tradeBuffer,
                 int timestep,
                 LatencyHistogram* hist,
                 const TransactionCostModel* costModel) {

    while (__builtin_expect(!lob.bids.empty() && !lob.asks.empty(), 0)) {

        auto bidIt = lob.bids.begin();
        auto askIt = lob.asks.begin();

        while (!bidIt->second.empty() && bidIt->second.front().quantity == 0)
            bidIt->second.pop_front();
        if (__builtin_expect(bidIt->second.empty(), 0)) { lob.bids.erase(bidIt); continue; }

        while (!askIt->second.empty() && askIt->second.front().quantity == 0)
            askIt->second.pop_front();
        if (__builtin_expect(askIt->second.empty(), 0)) { lob.asks.erase(askIt); continue; }

        if (__builtin_expect(bidIt->first >= askIt->first, 1)) {

            Order& buy  = bidIt->second.front();
            Order& sell = askIt->second.front();

            int& bidFilled = lob.bidVolumeFilled[buy.price];
            int& askFilled = lob.askVolumeFilled[sell.price];

            bool buyReady  = (bidFilled >= buy.queuePos);
            bool sellReady = (askFilled >= sell.queuePos);

            if (__builtin_expect(!buyReady || !sellReady, 0)) {
                bidFilled += buy.quantity;
                askFilled += sell.quantity;
                break;
            }

            int    qty   = std::min(buy.quantity, sell.quantity);
            double price = sell.price;
            double lob_spread = lob.spread();

            tradeBuffer += std::to_string(timestep) + ','
                         + buy.agent  + ','
                         + sell.agent + ','
                         + std::to_string(price) + ','
                         + std::to_string(qty)   + '\n';

            if (Bot* buyer = findBot(bots, buy.agent)) {
                buyer->recordTrade(price, qty, true);
                if (costModel) {
                    auto cost = costModel->compute(price, qty, true, buyer->position, lob_spread);
                    buyer->cash -= cost.total();
                    buyer->totalCostPaid += cost.total();
                }
            }
            if (Bot* seller = findBot(bots, sell.agent)) {
                seller->recordTrade(price, qty, false);
                if (costModel) {
                    auto cost = costModel->compute(price, qty, false, seller->position, lob_spread);
                    seller->cash -= cost.total();
                    seller->totalCostPaid += cost.total();
                }
            }

            uint64_t tsc_filled = rdtsc();
            if (hist) {
                uint64_t tsc_created = std::min(buy.tsc_created, sell.tsc_created);
                if (tsc_created > 0)
                    hist->record(tsc_created, tsc_filled);
            }

            bidFilled += qty;
            askFilled += qty;
            buy.quantity  -= qty;
            sell.quantity -= qty;

            if (__builtin_expect(buy.quantity == 0, 1))  { lob.orderIndex.erase(buy.id);  bidIt->second.pop_front(); }
            if (__builtin_expect(sell.quantity == 0, 1)) { lob.orderIndex.erase(sell.id); askIt->second.pop_front(); }
            if (bidIt->second.empty()) lob.bids.erase(bidIt);
            if (askIt->second.empty()) lob.asks.erase(askIt);

        } else {
            break;
        }
    }
}