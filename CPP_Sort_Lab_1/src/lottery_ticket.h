#pragma once
#include <string>
#include <cstdint>
#include <iosfwd>

struct LotteryTicket 
{
    std::string ticketNumber;
    int nominalValue{};
    std::int32_t lotteryDate{};
    long long winAmount{};

    LotteryTicket() = default;
    LotteryTicket(std::string t, int n, std::int32_t d, long long w)
        : ticketNumber(std::move(t)), nominalValue(n), lotteryDate(d), winAmount(w) {}
};

inline int compareTickets(const LotteryTicket& a, const LotteryTicket& b) noexcept
{
    if (a.lotteryDate != b.lotteryDate)
        return (a.lotteryDate < b.lotteryDate) ? -1 : 1;
    if (a.winAmount != b.winAmount)
        return (a.winAmount > b.winAmount) ? -1 : 1;
    if (a.ticketNumber != b.ticketNumber)
        return (a.ticketNumber < b.ticketNumber) ? -1 : 1;
    return 0;
}

inline bool operator< (const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) <  0; }
inline bool operator> (const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) >  0; }
inline bool operator<=(const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) <= 0; }
inline bool operator>=(const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) >= 0; }
inline bool operator==(const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) == 0; }
inline bool operator!=(const LotteryTicket& a, const LotteryTicket& b) noexcept { return compareTickets(a, b) != 0; }
