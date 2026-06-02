#pragma once
#include <atomic>
#include <limits>
#include <string>
#include <cstdint>

enum class Side { BUY, SELL };

struct alignas(64) Order {
    int         id       = 0;
    double      price    = 0.0;
    int         quantity = 0;
    Side        side     = Side::BUY;
    int         queuePos = 0;
    uint64_t    tsc_created = 0;
    char        agent[32] = {};

    static std::atomic<int> nextId;

    static Order makeLimitOrder(const char* agentName, double price, int quantity, Side side);
    static Order makeMarketOrder(const char* agentName, int quantity, Side side);

    static Order makeLimitOrder(const std::string& agentName, double price, int quantity, Side side) {
        return makeLimitOrder(agentName.c_str(), price, quantity, side);
    }
    static Order makeMarketOrder(const std::string& agentName, int quantity, Side side) {
        return makeMarketOrder(agentName.c_str(), quantity, side);
    }
};

inline std::atomic<int> Order::nextId{0};