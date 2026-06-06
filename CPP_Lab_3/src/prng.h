#pragma once

#include <cstdint>

namespace prng {

class IGenerator {
public:
    virtual ~IGenerator() = default;
    virtual std::uint32_t next() = 0;
    virtual const char*   name() const = 0;
};


class LcgXor : public IGenerator {
public:
    explicit LcgXor(std::uint64_t seed = 0xDEADBEEFCAFEBABEull) : state_(seed | 1) {}
    std::uint32_t next() override {
        state_ = state_ * 6364136223846793005ull + 1442695040888963407ull;
        std::uint32_t hi = static_cast<std::uint32_t>(state_ >> 32);
        std::uint32_t lo = static_cast<std::uint32_t>(state_);
        return hi ^ (lo * 0x9E3779B1u);
    }
    const char* name() const override { return "LCG_Xor"; }
private:
    std::uint64_t state_;
};

class MidSquareImproved : public IGenerator {
public:
    explicit MidSquareImproved(std::uint64_t seed = 0xABCDEF1234567890ull)
        : state_((seed & 0xFFFFFFFFull) | 1), counter_(0) {}

    std::uint32_t next() override {
        std::uint64_t sq = state_ * state_;
        std::uint32_t mid = static_cast<std::uint32_t>((sq >> 16) & 0xFFFFFFFFull);
        counter_ += 0xB5297A4Dull;
        std::uint32_t result = mid ^ static_cast<std::uint32_t>(counter_);
        state_ = result ? result : 1; 
        return result;
    }
    const char* name() const override { return "MidSquare_Improved"; }
private:
    std::uint64_t state_;
    std::uint64_t counter_;
};

class XorShift32StarPlus : public IGenerator {
public:
    explicit XorShift32StarPlus(std::uint32_t seed = 0x12345678u)
        : state_(seed ? seed : 0x12345678u), counter_(0) {}

    std::uint32_t next() override {
        std::uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        counter_ += 0x6D2B79F5u;
        return (x * 0x9E3779B9u) + counter_;
    }
    const char* name() const override { return "XorShift32_StarPlus"; }
private:
    std::uint32_t state_;
    std::uint32_t counter_;
};

} 
