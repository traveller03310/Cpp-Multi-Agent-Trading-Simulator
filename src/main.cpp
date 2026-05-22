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

static void printPnL(const std::string& instrument,
                     const std::vector<std::unique_ptr<Bot>>& bots,
                     double lastPrice) {
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "║          P&L SUMMARY — " << std::left << std::setw(29) << instrument << "║\n";
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

static void runInstrument(const std::string& name,
                          const std::string& csvPath,
                          int limit = 500) {
    MarketData data;
    if (!data.loadCSV(csvPath)) {
        std::cerr << "Failed to load " << csvPath << '\n';
        return;
    }

    LimitOrderBook lob;
    auto bots = makeBots();

    std::string tradeBuffer = "timestep,buyer,seller,price,quantity\n";
    std::string priceBuffer = "timestep,price\n";
    int    timestep  = 0;
    double lastPrice = 0.0;

    auto simStart = Clock::now();

    while (data.hasNext() && timestep < limit) {
        auto tick = data.next();
        lastPrice = tick.price;
        timestep++;

        priceBuffer += std::to_string(timestep) + ',' + std::to_string(tick.price) + '\n';

        for (auto& bot : bots)
            bot->onPriceUpdate(tick.price, lob, timestep);

        matchOrders(lob, bots, tradeBuffer, timestep);
    }

    auto simEnd = Clock::now();
    auto simUs  = std::chrono::duration_cast<std::chrono::microseconds>(simEnd - simStart).count();

    printPnL(name, bots, lastPrice);

    std::ofstream("data/trade_log_" + name + ".csv") << tradeBuffer;
    std::ofstream("data/price_log_" + name + ".csv") << priceBuffer;

    std::cout << "  Sim time : " << simUs << " µs  |  "
              << std::fixed << std::setprecision(0)
              << (timestep / (simUs / 1e6)) << " ticks/sec\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cout << "\n=== Multi-Instrument Trading Simulator ===\n";
    std::cout << "Running ETH and BTC in parallel order books...\n";

    auto wallStart = Clock::now();

    // Each instrument gets its own independent order book and bot fleet.
    // Adding a new instrument = one more runInstrument() call.
    runInstrument("ETH", "data/eth_1m.csv");
    runInstrument("BTC", "data/btc_1m.csv");

    auto wallUs = std::chrono::duration_cast<std::chrono::microseconds>(
                      Clock::now() - wallStart).count();

    std::cout << "\n── Total wall time: " << wallUs / 1000.0
              << " ms ───────────────────────\n";
    std::cout << "Logs: data/trade_log_ETH.csv, data/trade_log_BTC.csv\n";

    return 0;
}