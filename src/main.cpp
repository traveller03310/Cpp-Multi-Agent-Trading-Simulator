#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>

#include "orderbook.hpp"
#include "matching_engine.hpp"
#include "market_data.hpp"
#include "threaded_sim.hpp"

#include "../agents/random_bot.hpp"
#include "../agents/momentum_bot.hpp"
#include "../agents/rsi_bot.hpp"

using Clock = std::chrono::high_resolution_clock;

static std::vector<std::unique_ptr<Bot>> makeBots() {
    std::vector<std::unique_ptr<Bot>> bots;
    bots.push_back(std::make_unique<RandomBot>("BotA"));
    bots.push_back(std::make_unique<RandomBot>("BotB"));
    bots.push_back(std::make_unique<RandomBot>("BotC"));
    bots.push_back(std::make_unique<MomentumBot>("MomBot1",  5));
    bots.push_back(std::make_unique<MomentumBot>("MomBot2", 10));
    bots.push_back(std::make_unique<RSIBot>("RSIBot1", 14, 30.0, 70.0));
    return bots;
}

static void printPnL(const std::string& label,
                     const std::vector<std::unique_ptr<Bot>>& bots,
                     double lastPrice) {
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  P&L — " << std::left << std::setw(45) << label << "║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "║ Last price: $" << std::setw(10) << lastPrice
              << "                              ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout <<   "║  Bot          Cash($)      Pos    RealizedPnL($)    ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    for (const auto& bot : bots) {
        std::cout << "║  " << std::left  << std::setw(12) << bot->name
                  << std::right << std::setw(11) << bot->cash
                  << std::setw(6)  << bot->position
                  << std::setw(16) << bot->realizedPnl
                  << "    ║\n";
    }
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";
}

static long long runSingleThreaded(const std::string& name,
                                   const std::string& csvPath,
                                   std::vector<std::unique_ptr<Bot>>& bots,
                                   double& lastPrice,
                                   int limit = 500) {
    MarketData data;
    data.loadCSV(csvPath);

    LimitOrderBook lob;
    std::string tradeBuffer = "timestep,buyer,seller,price,quantity\n";
    std::string priceBuffer = "timestep,price\n";
    int timestep = 0;

    auto start = Clock::now();

    while (data.hasNext() && timestep < limit) {
        auto tick = data.next();
        lastPrice = tick.price;
        timestep++;
        priceBuffer += std::to_string(timestep) + ',' + std::to_string(tick.price) + '\n';
        for (auto& bot : bots)
            bot->onPriceUpdate(tick.price, lob, timestep);
        matchOrders(lob, bots, tradeBuffer, timestep);
    }

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  Clock::now() - start).count();

    std::ofstream("data/trade_log_" + name + ".csv") << tradeBuffer;
    std::ofstream("data/price_log_" + name + ".csv") << priceBuffer;

    printPnL(name + " (single-threaded)", bots, lastPrice);
    std::cout << "  Ticks: " << timestep
              << "  |  Time: " << us << " µs"
              << "  |  Throughput: " << std::fixed << std::setprecision(0)
              << (timestep / (us / 1e6)) << " ticks/sec\n";
    return us;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    constexpr int LIMIT = 500;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║     C++ Multi-Agent Trading Simulator                ║\n";
    std::cout <<   "║     Single-threaded  vs  4-Thread Pipeline           ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";

    std::cout << "\n━━━ SINGLE-THREADED ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    double ethPrice = 0.0, btcPrice = 0.0;
    long long stEth, stBtc;
    { auto bots = makeBots(); stEth = runSingleThreaded("ETH", "data/eth_1m.csv", bots, ethPrice, LIMIT); }
    { auto bots = makeBots(); stBtc = runSingleThreaded("BTC", "data/btc_1m.csv", bots, btcPrice, LIMIT); }

    std::cout << "\n━━━ MULTITHREADED (4-thread SPSC pipeline) ━━━━━━━━━━━━\n";
    long long mtEth, mtBtc;
    {
        auto bots = makeBots();
        ThreadedSim sim("ETH", "data/eth_1m.csv", bots, LIMIT);
        mtEth = sim.run();
        std::cout << "\n  ETH threaded:"
                  << "  ticks="  << sim.ticksProcessed()
                  << "  orders=" << sim.ordersPlaced()
                  << "  trades=" << sim.tradesExecuted()
                  << "  time="   << mtEth << " µs\n";
    }
    {
        auto bots = makeBots();
        ThreadedSim sim("BTC", "data/btc_1m.csv", bots, LIMIT);
        mtBtc = sim.run();
        std::cout << "\n  BTC threaded:"
                  << "  ticks="  << sim.ticksProcessed()
                  << "  orders=" << sim.ordersPlaced()
                  << "  trades=" << sim.tradesExecuted()
                  << "  time="   << mtBtc << " µs\n";
    }

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║        SINGLE-THREADED vs MULTITHREADED              ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "║  ETH  single: " << std::setw(8) << stEth << " µs"
              << "   multi: " << std::setw(8) << mtEth << " µs      ║\n";
    std::cout << "║  BTC  single: " << std::setw(8) << stBtc << " µs"
              << "   multi: " << std::setw(8) << mtBtc << " µs      ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout <<   "║  Architecture: Feed→Bots→Matcher via lock-free SPSC  ║\n";
    std::cout <<   "║  Logger:       async Thread 4, never blocks matcher   ║\n";
    std::cout <<   "║  False sharing: eliminated via alignas(64) padding    ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";

    return 0;
}