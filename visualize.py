import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import os

# ── Load data ──────────────────────────────────────────────
trades = pd.read_csv("data/trade_log.csv")
prices = pd.read_csv("data/price_log.csv")

bots = list(set(trades["buyer"].tolist() + trades["seller"].tolist()))

# ── Per-bot PnL ────────────────────────────────────────────
pnl = {bot: [] for bot in bots}
cumulative_pnl = {bot: 0.0 for bot in bots}

for _, row in trades.iterrows():
    buyer, seller, price, qty = row["buyer"], row["seller"], row["price"], row["quantity"]
    # buyer pays price, seller receives price
    cumulative_pnl[buyer]  -= price * qty
    cumulative_pnl[seller] += price * qty
    for bot in bots:
        pnl[bot].append(cumulative_pnl[bot])

pnl_df = pd.DataFrame(pnl)

# ── Metrics ────────────────────────────────────────────────
def sharpe_ratio(pnl_series):
    returns = pd.Series(pnl_series).diff().dropna()
    if returns.std() == 0:
        return 0
    return (returns.mean() / returns.std()) * np.sqrt(252)

def max_drawdown(pnl_series):
    s = pd.Series(pnl_series)
    peak = s.cummax()
    dd = s - peak
    return dd.min()

def win_rate(bot, trades_df):
    wins = 0
    total = 0
    for _, row in trades_df.iterrows():
        if row["seller"] == bot:
            wins += 1
            total += 1
        elif row["buyer"] == bot:
            total += 1
    return (wins / total * 100) if total > 0 else 0

metrics = []
for bot in bots:
    metrics.append({
        "Bot": bot,
        "Final PnL": round(cumulative_pnl[bot], 2),
        "Sharpe Ratio": round(sharpe_ratio(pnl[bot]), 3),
        "Max Drawdown": round(max_drawdown(pnl[bot]), 2),
        "Win Rate (%)": round(win_rate(bot, trades), 1),
        "Total Trades": len(trades[
            (trades["buyer"] == bot) | (trades["seller"] == bot)
        ])
    })

metrics_df = pd.DataFrame(metrics).sort_values("Final PnL", ascending=False)
print("\n===== PERFORMANCE METRICS =====")
print(metrics_df.to_string(index=False))

# ── Plot ───────────────────────────────────────────────────
fig = plt.figure(figsize=(18, 12))
fig.suptitle("Multi-Agent Trading Simulator — Performance Dashboard", 
             fontsize=16, fontweight="bold", y=0.98)

gs = gridspec.GridSpec(3, 3, figure=fig, hspace=0.45, wspace=0.35)

colors = ["#2196F3", "#4CAF50", "#FF5722", "#9C27B0"]

# 1. ETH Price
ax1 = fig.add_subplot(gs[0, :])
ax1.plot(prices["timestep"], prices["price"], color="#FF9800", linewidth=1.5)
ax1.set_title("ETH Market Price Over Time")
ax1.set_xlabel("Timestep")
ax1.set_ylabel("Price (USDT)")
ax1.grid(True, alpha=0.3)

# 2. Cumulative PnL
ax2 = fig.add_subplot(gs[1, :2])
for i, bot in enumerate(bots):
    ax2.plot(pnl_df[bot], label=bot, color=colors[i % len(colors)], linewidth=1.8)
ax2.axhline(0, color="gray", linestyle="--", linewidth=0.8)
ax2.set_title("Cumulative PnL per Bot")
ax2.set_xlabel("Trade #")
ax2.set_ylabel("PnL (USDT)")
ax2.legend()
ax2.grid(True, alpha=0.3)

# 3. Final PnL bar chart
ax3 = fig.add_subplot(gs[1, 2])
bar_colors = [colors[i % len(colors)] for i in range(len(metrics_df))]
bars = ax3.bar(metrics_df["Bot"], metrics_df["Final PnL"], color=bar_colors)
ax3.axhline(0, color="gray", linestyle="--", linewidth=0.8)
ax3.set_title("Final PnL Comparison")
ax3.set_ylabel("PnL (USDT)")
for bar, val in zip(bars, metrics_df["Final PnL"]):
    ax3.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
             f"{val:.0f}", ha="center", va="bottom", fontsize=9)

# 4. Sharpe Ratio
ax4 = fig.add_subplot(gs[2, 0])
ax4.bar(metrics_df["Bot"], metrics_df["Sharpe Ratio"], color=bar_colors)
ax4.set_title("Sharpe Ratio")
ax4.set_ylabel("Sharpe")
ax4.axhline(0, color="gray", linestyle="--", linewidth=0.8)
ax4.grid(True, alpha=0.3, axis="y")

# 5. Win Rate
ax5 = fig.add_subplot(gs[2, 1])
ax5.bar(metrics_df["Bot"], metrics_df["Win Rate (%)"], color=bar_colors)
ax5.set_title("Win Rate (%)")
ax5.set_ylabel("Win Rate %")
ax5.set_ylim(0, 100)
ax5.grid(True, alpha=0.3, axis="y")

# 6. Max Drawdown
ax6 = fig.add_subplot(gs[2, 2])
ax6.bar(metrics_df["Bot"], metrics_df["Max Drawdown"], color=bar_colors)
ax6.set_title("Max Drawdown")
ax6.set_ylabel("Drawdown (USDT)")
ax6.grid(True, alpha=0.3, axis="y")

plt.savefig("data/performance_dashboard.png", dpi=150, bbox_inches="tight")
plt.show()
print("\nChart saved to data/performance_dashboard.png")