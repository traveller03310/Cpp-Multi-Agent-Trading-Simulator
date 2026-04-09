#pragma once
#include <atomic>  // thread-safe counter for order IDs (no race conditions)
#include <limits>  // gives std::numeric_limits<double>::max() (largest double value)
#include <string>

// enum class prevents accidental implicit conversion to int (safer than plain enum)
enum class Side {
    BUY,
    SELL
};

struct Order {
    int id;
    std::string agent;
    double price;
    int quantity;
    Side side;    // BUY or SELL

    // Shared counter across all Order instances
    // atomic = thread-safe increment — if two bots place orders simultaneously, IDs won't clash
    // static = belongs to the class, not any single object
    static std::atomic<int> nextId;

    // nextId++ → atomically grab current ID then increment (thread-safe)
    // std::move(agent) → moves string instead of copying (performance optimization)
    // Returns a fully constructed Order struct
    static Order makeLimitOrder(std::string agent, double price, int quantity, Side side) {
        return { nextId++, std::move(agent), price, quantity, side };
    }

    static Order makeMarketOrder(std::string agent, int quantity, Side side) {
        double p = (side == Side::BUY) ? std::numeric_limits<double>::max() : 0.0;
        return { nextId++, std::move(agent), p, quantity, side };
    }
};

inline std::atomic<int> Order::nextId{0};

// Order::nextId (shared atomic counter)
//        ↓
// makeLimitOrder()  → price = exact value    → sits in order book
// makeMarketOrder() → price = MAX or 0.0     → matches immediately

// Both use nextId++ atomically:
//   Thread 1: bot_A places order → id=0
//   Thread 2: bot_B places order → id=1   (no clash, atomic guarantees this)