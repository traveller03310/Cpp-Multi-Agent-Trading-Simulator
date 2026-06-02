#pragma once
#include <time.h>
#include <cstdint>

// rdtsc: CPU cycle counter, ~0.3ns overhead vs ~25ns for chrono syscall.
// Used to timestamp every pipeline stage: created → matched → filled → logged.

#if defined(__x86_64__) || defined(__i386__)
inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<uint64_t>(hi) << 32) | lo;
}
#else
// ARM / Apple Silicon fallback
inline uint64_t rdtsc() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + ts.tv_nsec;
}
#endif

struct TscClock {
    double ns_per_cycle = 0.333;

    // Calibrate by correlating TSC with CLOCK_MONOTONIC over 10ms
    void calibrate() {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        uint64_t r0 = rdtsc();
        do { clock_gettime(CLOCK_MONOTONIC, &t1); }
        while ((t1.tv_sec - t0.tv_sec) * 1'000'000'000LL +
               (t1.tv_nsec - t0.tv_nsec) < 10'000'000LL);
        uint64_t r1 = rdtsc();
        double elapsed_ns = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
        ns_per_cycle = elapsed_ns / static_cast<double>(r1 - r0);
    }

    double to_ns(uint64_t cycles) const { return cycles * ns_per_cycle; }
};

inline TscClock& globalClock() {
    static TscClock clk;
    return clk;
}