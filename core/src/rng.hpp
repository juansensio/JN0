#pragma once
#include <cstdint>
namespace janus::detail {
class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) : state_(seed) {}
    std::uint64_t next() {
        auto z = (state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
    // Callers supply n > 0. Unsigned subtraction implements 2^64 - n.
    std::uint64_t bounded(std::uint64_t n) {
        const auto threshold = (std::uint64_t{0} - n) % n;
        for (;;) { const auto x = next(); if (x >= threshold) return x % n; }
    }
private:
    std::uint64_t state_;
};
}
