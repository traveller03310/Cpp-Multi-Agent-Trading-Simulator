#include "httplib.h"
#include "orderbook.hpp"
#include "flat_orderbook.hpp"
#include "order.hpp"
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include <sstream>

// simple JSON parser helpers
static std::string getStr(const std::string& body, const std::string& key) {
    auto pos = body.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = body.find(":", pos);
    pos = body.find("\"", pos);
    auto end = body.find("\"", pos + 1);
    return body.substr(pos + 1, end - pos - 1);
}

static double getDbl(const std::string& body, const std::string& key) {
    auto pos = body.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0.0;
    pos = body.find(":", pos) + 1;
    while (body[pos] == ' ') pos++;
    return std::stod(body.substr(pos));
}

static int getInt(const std::string& body, const std::string& key) {
    return (int)getDbl(body, key);
}

int main() {
    LimitOrderBook lob;
    std::mutex lobMutex;
    std::atomic<int> orderCount{0};

    httplib::Server svr;

    // POST /order — accept limit, market, cancel orders
    svr.Post("/order", [&](const httplib::Request& req, httplib::Response& res) {
        std::string body = req.body;
        std::string type = getStr(body, "type");
        std::string side = getStr(body, "side");
        double price     = getDbl(body, "price");
        int    quantity  = getInt(body, "quantity");
        int    botID     = getInt(body, "bot_id");

        std::string botName = "bot_" + std::to_string(botID);
        int orderId = ++orderCount;

        {
            std::lock_guard<std::mutex> lock(lobMutex);

            if (type == "limit") {
                Side s = (side == "buy") ? Side::BUY : Side::SELL;
                lob.addOrder(Order::makeLimitOrder(botName, price, quantity, s));
            } else if (type == "market") {
                // market order: extreme price to guarantee fill
                Side s = (side == "buy") ? Side::BUY : Side::SELL;
                double marketPrice = (s == Side::BUY)
                    ? std::numeric_limits<double>::max()
                    : 0.0;
                lob.addOrder(Order::makeLimitOrder(botName, marketPrice, quantity, s));
            }
            // cancel: just acknowledge for now
        }

        std::string json = "{\"status\":\"accepted\",\"order_id\":" +
                           std::to_string(orderId) + "}";
        res.set_content(json, "application/json");
    });

    // GET /orderbook — return best bid/ask
    svr.Get("/orderbook", [&](const httplib::Request&, httplib::Response& res) {
        double bestBid = 0.0, bestAsk = 0.0;

        {
            std::lock_guard<std::mutex> lock(lobMutex);
            if (!lob.bids.empty())
                bestBid = lob.bids.begin()->first;
            if (!lob.asks.empty())
                bestAsk = lob.asks.begin()->first;
        }

        std::ostringstream oss;
        oss << std::fixed;
        oss << "{\"best_bid\":" << bestBid
            << ",\"best_ask\":" << bestAsk
            << ",\"spread\":"   << (bestAsk - bestBid) << "}";

        res.set_content(oss.str(), "application/json");
    });

    std::cout << "C++ Trading Engine HTTP Server running on :8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}