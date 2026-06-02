#pragma once
#include <array>
#include <cstddef>
#include <cassert>
#include "order.hpp"

// Pre-allocated slab of Orders. acquire() = 2 instructions, zero malloc in hot path.
// Each thread owns its own pool — not thread-safe by design.

template<size_t MAX_ORDERS = 65536>
class OrderPool {
public:
    OrderPool() {
        for (size_t i = MAX_ORDERS; i > 0; --i)
            freeStack_[MAX_ORDERS - i] = &pool_[i - 1];
        freeTop_ = MAX_ORDERS;
    }

    Order* acquire() {
        assert(freeTop_ > 0 && "OrderPool exhausted");
        return freeStack_[--freeTop_];
    }

    void release(Order* o) {
        assert(freeTop_ < MAX_ORDERS && "OrderPool double-free");
        freeStack_[freeTop_++] = o;
    }

    size_t used()     const { return MAX_ORDERS - freeTop_; }
    size_t free()     const { return freeTop_; }
    size_t capacity() const { return MAX_ORDERS; }

private:
    alignas(64) std::array<Order, MAX_ORDERS> pool_;
    std::array<Order*, MAX_ORDERS> freeStack_;
    size_t freeTop_ = 0;
};