# C++-Multi-Agent-Trading-Simulator
A C++ market simulation engine with a limit order book and matching engine for experimenting with algorithmic trading strategies.

## Overview

This project implements a **simplified cryptocurrency exchange simulator** written in C++. It models the internal systems used by modern trading platforms such as Binance and Coinbase.

Instead of real traders, the system runs **multiple autonomous trading agents** that interact with a simulated exchange. These agents place buy and sell orders which are processed by a **matching engine** and stored in a **limit order book**, just like in real financial markets.

The goal of the project is to explore:

* Market microstructure
* Algorithmic trading behavior
* Exchange architecture
* High-performance order matching systems

---

# System Architecture

The simulator models the core components of an electronic exchange.

```
                +-------------------+
                |   Trading Agents  |
                |  (Bots/Strategies)|
                +---------+---------+
                          |
                          v
                +-------------------+
                |   Matching Engine |
                |  (Order Matching) |
                +---------+---------+
                          |
                          v
                +-------------------+
                |   Limit Order Book|
                |  (Market State)   |
                +---------+---------+
                          |
                          v
                +-------------------+
                |   Trade Execution |
                |   & Market Data   |
                +-------------------+
```

---

# Core Components

## 1. Trading Agents

Agents simulate automated traders interacting with the market. Each agent follows a strategy and submits orders to the exchange.

Example agent behaviors:

**Random Trader**

* Submits random buy/sell orders.

**Market Maker**

* Provides liquidity by placing buy and sell orders around the current price.

**Momentum Trader**

* Buys when price increases.
* Sells when price decreases.

**Liquidity Trader**

* Executes larger market orders.

Example agent actions:

```
Agent 1 → BUY 1 BTC @ 50000
Agent 2 → SELL 2 BTC @ 50200
Agent 3 → BUY 3 BTC @ 49950
```

These agents interact with each other to generate a dynamic simulated market.

---

# 2. Limit Order Book

The **Limit Order Book (LOB)** stores all active buy and sell orders.

Example state of the order book:

```
BUY ORDERS (Bids)
Price     Quantity
50000     3
49950     2

SELL ORDERS (Asks)
Price     Quantity
50100     1
50200     4
```

Important concepts:

| Term     | Meaning                                  |
| -------- | ---------------------------------------- |
| Bid      | A buy order                              |
| Ask      | A sell order                             |
| Best Bid | Highest buy price                        |
| Best Ask | Lowest sell price                        |
| Spread   | Difference between best bid and best ask |

The order book continuously updates as new orders arrive or trades execute.

---

# 3. Matching Engine

The **matching engine** processes incoming orders and determines when trades occur.

A trade executes when:

```
Buy Price ≥ Sell Price
```

Example:

```
Agent A: BUY 1 BTC @ 50100
Agent B: SELL 1 BTC @ 50100
```

Trade result:

```
Trade Executed
Price: 50100
Quantity: 1 BTC
```

The engine updates the order book and records the trade.

---

# 4. Trade Execution

When orders match, the simulator generates a trade event.

Example output:

```
Trade executed
Price: 50000
Quantity: 1 BTC
Buyer: Agent 3
Seller: Agent 7
```

Trade data allows the simulator to track:

* price movement
* trade volume
* market liquidity
* agent behavior

---

# Example Simulation Output

```
Agent 1 placed BUY 1 @ 50000
Agent 2 placed SELL 1 @ 50000

Trade Executed
Price: 50000
Quantity: 1

Order Book:
Best Bid: 49950
Best Ask: 50100
```

---

# Project Structure

```
crypto-trading-simulator
│
├── src
│   ├── main.cpp
│   ├── order.hpp
│   ├── orderbook.hpp
│   ├── orderbook.cpp
│   ├── matching_engine.hpp
│   └── matching_engine.cpp
│
├── agents
│   ├── random_agent.cpp
│   ├── market_maker.cpp
│   └── momentum_trader.cpp
│
├── docs
│   └── architecture.md
│
├── build
│
├── README.md
└── .gitignore
```

---

# Technologies

* **Language:** C++
* **Concepts Used:**

  * Object-oriented design
  * Data structures
  * Simulation modeling
  * Event-driven systems
  * Algorithmic trading concepts

---

# Possible Extensions

This simulator can be extended with more advanced features such as:

* market impact modeling
* order cancellation
* latency simulation
* multi-asset trading
* visual order book display
* historical market replay
* statistical market analysis

---

# Learning Outcomes

By building this project, you gain experience with:

* exchange architecture
* algorithmic trading systems
* efficient C++ data structures
* multi-agent simulations
* financial market mechanics

---

# Inspiration

Real trading firms and exchanges run highly optimized versions of these systems. Firms such as Citadel Securities, Jane Street, and Hudson River Trading build extremely low-latency matching engines capable of processing millions of orders per second.

This project is an educational, simplified version of those systems.

---

# Author

Built as a learning project exploring financial markets, exchange infrastructure, and high-performance systems programming in C++.

