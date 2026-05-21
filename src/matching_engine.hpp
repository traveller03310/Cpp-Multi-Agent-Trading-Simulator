#pragma once
#include "orderbook.hpp"
#include "../agents/bot.hpp"
#include <string>
#include <vector>
#include <memory>

void matchOrders(LimitOrderBook &lob,
                 std::vector<std::unique_ptr<Bot>> &bots,
                 std::string &tradeBuffer,
                 int timestep);