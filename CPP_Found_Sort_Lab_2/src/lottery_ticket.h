#pragma once

#include <string>
#include <iosfwd>
#include <cstdint>

struct LotteryTicket 
{
    std::string ticketNumber;
    int         nominalValue{};
    std::string lotteryDate;
    long long   winAmount{};

    LotteryTicket() = default;
    LotteryTicket(std::string t, int n, std::string d, long long w)
        : ticketNumber(std::move(t)), nominalValue(n), lotteryDate(std::move(d)), winAmount(w) {}

    bool operator==(const LotteryTicket& o) const noexcept 
    {
        return ticketNumber == o.ticketNumber && nominalValue == o.nominalValue
            && lotteryDate == o.lotteryDate  && winAmount    == o.winAmount;
    }
};
