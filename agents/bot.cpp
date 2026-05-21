#include "bot.hpp"
#include "../src/orderbook.hpp"
#include "../src/flat_orderbook.hpp"

void Bot::onPriceUpdate(double, LimitOrderBook&, int) {}
void Bot::onPriceUpdate(double, FlatOrderBook&,  int) {}