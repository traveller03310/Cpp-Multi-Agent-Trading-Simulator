#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <chrono>

#include "orderbook.hpp"
#include "matching_engine.hpp"
#include "market_data.hpp"

#include "../agents/random_bot.hpp"
#include "../agents/momentum_bot.hpp"
#include "../agents/rsi_bot.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto wallStart = std::chrono::high_resolution_clock::now();

    MarketData data;
    data.loadCSV("data/eth_1m.csv");

    LimitOrderBook lob;

    std::vector<std::unique_ptr<Bot>> bots;
    bots.push_back(std::make_unique<RandomBot>("BotA"));
    bots.push_back(std::make_unique<RandomBot>("BotB"));
    bots.push_back(std::make_unique<RandomBot>("BotC"));
    bots.push_back(std::make_unique<MomentumBot>("MomBot1",  5));
    bots.push_back(std::make_unique<MomentumBot>("MomBot2", 10));
    bots.push_back(std::make_unique<RSIBot>("RSIBot1", 14, 30.0, 70.0));

    std::string tradeBuffer = "timestep,buyer,seller,price,quantity\n";
    std::string priceBuffer = "timestep,price\n";

    int    timestep  = 0;
    double lastPrice = 0.0;

    auto simStart = std::chrono::high_resolution_clock::now();

    while (data.hasNext()) {
        auto tick = data.next();
        lastPrice = tick.price;
        timestep++;

        std::cout << "\n=== Timestep " << timestep << " ===\n"
                  << "Price: " << tick.price << '\n';

        priceBuffer += std::to_string(timestep) + ',' + std::to_string(tick.price) + '\n';

        for (auto& bot : bots)
            bot->onPriceUpdate(tick.price, lob, timestep);

        matchOrders(lob, bots, tradeBuffer, timestep);

        if (timestep > 500) break;
    }

    auto simEnd = std::chrono::high_resolution_clock::now();

    std::ofstream("data/trade_log.csv")  << tradeBuffer;
    std::ofstream("data/price_log.csv")  << priceBuffer;

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║                 FINAL P&L SUMMARY                   ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "║ Last price: $" << std::setw(10) << lastPrice
              << "                              ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";
    std::cout <<   "║  Bot          Cash($)    Position   MTM P&L($)      ║\n";
    std::cout <<   "╠══════════════════════════════════════════════════════╣\n";

    for (const auto& bot : bots) {
        double mtm = bot->pnl(lastPrice);
        std::cout << "║  " << std::left  << std::setw(12) << bot->name
                  << std::right << std::setw(10) << bot->cash
                  << "   " << std::setw(6) << bot->position
                  << "   " << std::setw(12) << mtm
                  << "      ║\n";
    }

    std::cout <<   "╚══════════════════════════════════════════════════════╝\n";

    auto wallEnd = std::chrono::high_resolution_clock::now();
    auto simUs   = std::chrono::duration_cast<std::chrono::microseconds>(simEnd  - simStart).count();
    auto wallUs  = std::chrono::duration_cast<std::chrono::microseconds>(wallEnd - wallStart).count();

    std::cout << "\n── Benchmarks ──────────────────────────────────────────\n";
    std::cout << "  Simulation loop : " << simUs  << " µs  (" << simUs  / 1000.0 << " ms)\n";
    std::cout << "  Total wall time : " << wallUs << " µs  (" << wallUs / 1000.0 << " ms)\n";
    std::cout << "  Timesteps run   : " << timestep << "\n";
    std::cout << "  Throughput      : " << std::fixed << std::setprecision(0)
              << (timestep / (simUs / 1e6)) << " ticks/sec\n";
    std::cout << "────────────────────────────────────────────────────────\n";

    std::cout << "\nLogs saved to data/trade_log.csv and data/price_log.csv\n";
    return 0;
}