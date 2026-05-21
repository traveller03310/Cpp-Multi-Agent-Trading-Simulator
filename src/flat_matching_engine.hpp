#pragma once
#include "flat_orderbook.hpp"
#include "../agents/bot.hpp"
#include <string>
#include <vector>
#include <memory>

void matchOrdersFlat(FlatOrderBook &lob,
                     std::vector<std::unique_ptr<Bot>> &bots,
                     std::string &tradeBuffer,
                     int timestep);