#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include "rdtsc.hpp"

struct LatencyHistogram {
    std::string label;
    std::vector<uint64_t> samples_cycles;

    explicit LatencyHistogram(std::string lbl) : label(std::move(lbl)) {
        samples_cycles.reserve(4096);
    }

    void record(uint64_t start_tsc, uint64_t end_tsc) {
        if (end_tsc > start_tsc)
            samples_cycles.push_back(end_tsc - start_tsc);
    }

    void print(const TscClock& clk = globalClock()) const {
        if (samples_cycles.empty()) {
            std::cout << "  [" << label << "] no samples\n";
            return;
        }
        auto sorted = samples_cycles;
        std::sort(sorted.begin(), sorted.end());

        auto pct = [&](double p) -> double {
            size_t idx = static_cast<size_t>(p / 100.0 * sorted.size());
            if (idx >= sorted.size()) idx = sorted.size() - 1;
            return clk.to_ns(sorted[idx]);
        };

        double avg_ns = clk.to_ns(
            std::accumulate(sorted.begin(), sorted.end(), uint64_t{0}) / sorted.size());

        std::cout << "\n── Latency: " << label << " ──────────────────────────────────\n";
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  samples : " << sorted.size()             << "\n";
        std::cout << "  min     : " << clk.to_ns(sorted.front()) << " ns\n";
        std::cout << "  avg     : " << avg_ns                    << " ns\n";
        std::cout << "  p50     : " << pct(50)                   << " ns\n";
        std::cout << "  p95     : " << pct(95)                   << " ns\n";
        std::cout << "  p99     : " << pct(99)                   << " ns\n";
        std::cout << "  p99.9   : " << pct(99.9)                 << " ns\n";
        std::cout << "  max     : " << clk.to_ns(sorted.back())  << " ns\n";
        std::cout << "────────────────────────────────────────────────────────\n";
    }
};