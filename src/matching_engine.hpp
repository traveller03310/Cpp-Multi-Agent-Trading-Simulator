#pragma once
#include "orderbook.hpp"
#include <fstream>

void matchOrders(LimitOrderBook &lob, std::ofstream &logFile, int timestep);