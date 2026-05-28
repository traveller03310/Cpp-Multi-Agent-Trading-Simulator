# C++ Multi-Agent Trading Simulator

A high-performance, event-driven trading simulator written in modern C++17. Multiple autonomous bots compete in a real limit order book using historical ETH/USDT and synthetic BTC/USDT market data. Built as a deep dive into systems programming concepts relevant to HFT and trading infrastructure.

---

## What This Project Demonstrates

| Concept | Where |
|---|---|
| Lock-free order ID generation | `std::atomic<int>` in `order.hpp` |
| Dangling pointer elimination | `std::list` stable nodes in `orderbook.hpp` |
| Cache-friendly order book | `FlatOrderBook` — sorted `std::vector` + binary search |
| Modern PRNG (no `rand()`) | `std::mt19937` + `std::uniform_int_distribution` in `random_bot.hpp` |
| RAII memory management | `std::unique_ptr<Bot>` throughout |
| Mark-to-market P&L | Per-bot cash, position, realizedPnl tracked live in C++ |
| chrono benchmarking | ns-precision timing on every run |
| Multi-instrument simulation | Independent order books per instrument |
| RSI strategy | Full 14-period RSI with 30/70 thresholds |
| Momentum strategy | Half-window average comparison (fixed signal) |

---

## Architecture

```
crypto-trading-simulator/
├── src/
│   ├── main.cpp                  # Simulation loop, benchmarks, P&L summary
│   ├── order.hpp                 # Order struct — atomic ID, Side enum, market/limit factory
│   ├── orderbook.hpp             # LimitOrderBook — std::map + std::list (stable pointers)
│   ├── orderbook_base.hpp        # Shared base for LOB types
│   ├── flat_orderbook.hpp        # FlatOrderBook — std::vector + binary search (cache-friendly)
│   ├── matching_engine.hpp/cpp   # Matches orders, executes trades, updates bot P&L
│   ├── flat_matching_engine.hpp/cpp  # Same for FlatOrderBook
│   ├── market_data.hpp/cpp       # Binance CSV parser
│   └── orderbook.cpp
├── agents/
│   ├── bot.hpp/cpp               # Base class — cash, position, realizedPnl, recordTrade()
│   ├── random_bot.hpp            # Noise trader — mt19937, uniform distribution
│   ├── momentum_bot.hpp/cpp      # Trend follower — half-window avg comparison
│   └── rsi_bot.hpp               # Mean reversion — 14-period RSI, buy <30 sell >70
├── data/
│   ├── eth_1m.csv                # Binance ETH/USDT 1m candles
│   ├── btc_1m.csv                # Synthetic BTC/USDT (same format)
│   ├── trade_log_ETH.csv         # Generated — all ETH trades
│   ├── trade_log_BTC.csv         # Generated — all BTC trades
│   ├── price_log_ETH.csv         # Generated — ETH price per tick
│   └── price_log_BTC.csv         # Generated — BTC price per tick
├── visualize.py                  # Performance dashboard (Python)
├── Makefile
└── README.md
```

---

## Trading Agents

### RandomBot
Noise trader. Places limit orders ±$5 from market price with 30% buy / 30% sell / 40% hold probability. Uses `std::mt19937` seeded from `std::random_device` — no `rand()`.

### MomentumBot
Trend follower. Maintains a rolling price window (configurable, default 5 ticks). Compares the average of the recent half vs the earlier half — buys on uptrend, sells on downtrend. More robust than naive back-vs-front comparison.

### RSIBot
Mean reversion. Computes 14-period RSI using standard avgGain/avgLoss formula. Buys when RSI < 30 (oversold), sells when RSI > 70 (overbought). Entirely stateless between instruments.

---

## P&L Tracking

Every bot tracks three values in real time, updated by the matching engine after each fill:

| Field | Meaning |
|---|---|
| `cash` | Net cash flow from all trades (starts at $100,000) |
| `position` | Net units held (positive = long, negative = short) |
| `realizedPnl` | Locked-in profit/loss from closed positions |

Mark-to-market P&L = `realizedPnl + (cash - 100000) + position * currentPrice`

---

## Key Engineering Decisions

### 1. `std::list` instead of `std::queue` for order storage
`std::queue` is backed by `std::deque`, which can reallocate and **invalidate every raw pointer** stored in `orderIndex`. `std::list` nodes never move in memory after insertion — `orderIndex` pointers stay valid forever. This eliminates a class of dangling pointer bugs under load.

### 2. `FlatOrderBook` — contiguous memory layout
`std::map` is a red-black tree — every insert/lookup chases heap-allocated pointers across memory, thrashing the cache. `FlatOrderBook` stores price levels in a sorted `std::vector`. All levels sit in contiguous memory; the CPU prefetcher works effectively. Binary search gives the same O(log n) complexity but with far lower constants at realistic price-level counts.

### 3. `mt19937` replacing `rand()`
`rand()` has implementation-defined behaviour, modulo bias with `% N`, and shared global state between threads. `std::mt19937` with `std::uniform_int_distribution` gives uniform distribution, a 2^19937-1 period, and per-instance state safe for multithreaded use.

### 4. Atomic order IDs
`static std::atomic<int> nextId` in `Order` gives lock-free unique IDs across all threads. Uses `memory_order_relaxed` — we only need atomicity, not ordering relative to other variables.

---

## Getting Started

### Prerequisites

**C++17 compiler:**
```bash
# Mac
xcode-select --install

# Ubuntu/Debian
sudo apt install g++ make
```

**Python (for visualization):**
```bash
pip3 install pandas matplotlib
```

### Build & Run

```bash
git clone https://github.com/stevie-x/Cpp-Multi-Agent-Trading-Simulator.git
cd Cpp-Multi-Agent-Trading-Simulator

# Generate synthetic BTC data
python3 << 'EOF'
import random
random.seed(42)
price = 65000.0
ts = 1769904000000000
with open("data/btc_1m.csv", "w") as f:
    for i in range(1500):
        change = random.gauss(0, 150) + (65000 - price) * 0.005
        price = max(60000, min(70000, price + change))
        high = price + abs(random.gauss(0, 80))
        low = price - abs(random.gauss(0, 80))
        vol = random.uniform(10, 80)
        f.write(f"{ts},{price:.2f},{high:.2f},{low:.2f},{price:.2f},{vol:.5f},{ts+59999999},0,0,0,0,0\n")
        ts += 60000000
EOF

# Build and run
make run
```

### Visualization
```bash
python3 visualize.py
```

---

## Sample Output

```
=== Multi-Instrument Trading Simulator ===
Running ETH and BTC in parallel order books...

╔══════════════════════════════════════════════════════╗
║          P&L SUMMARY — ETH                          ║
╠══════════════════════════════════════════════════════╣
║ Last price: $  2408.43                               ║
╠══════════════════════════════════════════════════════╣
║  Bot          Cash($)      Pos    RealizedPnL($)    ║
╠══════════════════════════════════════════════════════╣
║  BotA          -33484.06    55       143770.70    ║
║  BotB          102877.49    -1       190042.63    ║
║  MomBot1       199042.42   -41       531065.42    ║
║  MomBot2       232627.21   -55       569822.29    ║
║  RSIBot1       105486.35    -2        95187.01    ║
╚══════════════════════════════════════════════════════╝
  Sim time : 4699 µs  |  106406 ticks/sec
```

---

## Benchmarks

Measured on MacBook Air M2, 500 ticks, 6 bots, 2 instruments:

| Metric | Value |
|---|---|
| ETH sim throughput | ~270,000 ticks/sec |
| BTC sim throughput | ~276,000 ticks/sec |
| Total wall time (both instruments) | ~28 ms |
| Order ID generation | lock-free, `memory_order_relaxed` |

---

## Roadmap

- [x] Limit order book with price-time priority
- [x] Dangling pointer fix (`std::list` stable nodes)
- [x] P&L tracking per bot (cash, position, realizedPnl)
- [x] RSI bot (14-period, 30/70 thresholds)
- [x] Replace `rand()` with `mt19937`
- [x] `chrono` benchmarking (ns precision)
- [x] Fix MomentumBot signal (half-window avg)
- [x] `FlatOrderBook` — cache-friendly contiguous layout
- [x] Multi-instrument skeleton (ETH + BTC independent books)
- [ ] Multithreading — producer/consumer pipeline with `std::mutex` + `std::condition_variable`
- [ ] Lock-free order queue (`std::atomic` + CAS)
- [ ] False sharing elimination (cache line padding)
- [ ] Unit tests
- [ ] MACD / Bollinger Bands agents
- [ ] Position limits and stop-loss

---

## Built With

- **C++17** — core simulator
- **Python 3 / matplotlib** — visualization
- **Binance Public Data** — historical ETH market data

---

## License

MIT
