#pragma once
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include "order.hpp"

// FIX 4.2 parser + builder.
// Handles NewOrderSingle (35=D) and ExecutionReport (35=8).
// Uses '|' as separator (SOH=0x01 in production).

struct FixMessage {
    std::unordered_map<int, std::string> fields;
    char separator = '|';

    bool parse(const std::string& raw) {
        fields.clear();
        std::istringstream ss(raw);
        std::string token;
        while (std::getline(ss, token, separator)) {
            if (token.empty()) continue;
            auto eq = token.find('=');
            if (eq == std::string::npos) continue;
            int tag = std::stoi(token.substr(0, eq));
            fields[tag] = token.substr(eq + 1);
        }
        return !fields.empty() && fields.count(35);
    }

    std::string get(int tag, const std::string& def = "") const {
        auto it = fields.find(tag);
        return it != fields.end() ? it->second : def;
    }

    char        msgType()  const { return get(35).empty() ? '?' : get(35)[0]; }
    std::string clOrdId()  const { return get(11); }
    std::string symbol()   const { return get(55); }
    std::string sender()   const { return get(49); }
    double      price()    const { auto s = get(44); return s.empty() ? 0.0 : std::stod(s); }
    int         qty()      const { auto s = get(38); return s.empty() ? 0   : std::stoi(s); }
    Side        side()     const { return get(54) == "1" ? Side::BUY : Side::SELL; }
    bool        isMarket() const { return get(40) == "1"; }
    bool        isLimit()  const { return get(40) == "2"; }
};

struct FixBuilder {
    char sep = '|';

    std::string newOrderSingle(const std::string& clOrdId,
                               const std::string& sender,
                               const std::string& symbol,
                               Side side, double price, int qty) const {
        std::string body;
        body += "35=D"  + std::string(1, sep);
        body += "49=" + sender  + sep;
        body += "11=" + clOrdId + sep;
        body += "55=" + symbol  + sep;
        body += std::string("54=") + (side == Side::BUY ? "1" : "2") + sep;
        body += std::string("40=2") + sep;
        body += "44=" + std::to_string(price) + sep;
        body += "38=" + std::to_string(qty)   + sep;

        std::string msg = "8=FIX.4.2" + std::string(1, sep)
                        + "9=" + std::to_string(body.size()) + sep
                        + body;
        msg += "10=" + checksum(msg) + sep;
        return msg;
    }

    std::string executionReport(const std::string& clOrdId,
                                const std::string& sender,
                                double fillPrice, int cumQty,
                                char ordStatus = '2',
                                char execType  = '2') const {
        std::string body;
        body += "35=8" + std::string(1, sep);
        body += "49="  + sender  + sep;
        body += "11="  + clOrdId + sep;
        body += "150=" + std::string(1, execType)  + sep;
        body += "39="  + std::string(1, ordStatus) + sep;
        body += "6="   + std::to_string(fillPrice) + sep;
        body += "14="  + std::to_string(cumQty)    + sep;

        std::string msg = "8=FIX.4.2" + std::string(1, sep)
                        + "9=" + std::to_string(body.size()) + sep
                        + body;
        msg += "10=" + checksum(msg) + sep;
        return msg;
    }

private:
    std::string checksum(const std::string& msg) const {
        int sum = 0;
        for (unsigned char c : msg) sum += c;
        char buf[5];
        std::snprintf(buf, sizeof(buf), "%03d", sum % 256);
        return buf;
    }
};

inline Order fixToOrder(const FixMessage& msg) {
    if (msg.msgType() != 'D')
        throw std::runtime_error("FIX: expected NewOrderSingle (35=D)");
    if (msg.isLimit())
        return Order::makeLimitOrder(msg.sender(), msg.price(), msg.qty(), msg.side());
    else
        return Order::makeMarketOrder(msg.sender(), msg.qty(), msg.side());
}