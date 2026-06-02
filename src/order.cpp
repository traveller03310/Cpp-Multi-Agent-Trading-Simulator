#include "order.hpp"
#include "rdtsc.hpp"
#include <cstring>
#include <limits>

Order Order::makeLimitOrder(const char* agentName, double price, int quantity, Side side) {
    Order o;
    o.id       = nextId++;
    o.price    = price;
    o.quantity = quantity;
    o.side     = side;
    o.tsc_created = rdtsc();
    std::strncpy(o.agent, agentName, sizeof(o.agent) - 1);
    o.agent[sizeof(o.agent) - 1] = '\0';
    return o;
}

Order Order::makeMarketOrder(const char* agentName, int quantity, Side side) {
    double p = (side == Side::BUY) ? std::numeric_limits<double>::max() : 0.0;
    return makeLimitOrder(agentName, p, quantity, side);
}