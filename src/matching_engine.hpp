#pragma once
#include "orderbook.hpp"
#include "../agents/bot.hpp"
#include "latency_histogram.hpp"
#include "transaction_cost.hpp"
#include <string>
#include <vector>
#include <memory>

void matchOrders(LimitOrderBook& lob,
                 std::vector<std::unique_ptr<Bot>>& bots,
                 std::string& tradeBuffer,
                 int timestep,
                 LatencyHistogram* hist = nullptr,
                 const TransactionCostModel* costModel = nullptr);