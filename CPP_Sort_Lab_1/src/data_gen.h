#pragma once
#include "lottery_ticket.h"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <random>
#include <string>
#include <vector>

namespace gen 
{
    inline std::int32_t dateFromDayOffset(int days)
    {
        std::tm tm{};
        tm.tm_year = 2022 - 1900;
        tm.tm_mon = 0;
        tm.tm_mday = 1 + days;
        std::mktime(&tm);
        return (tm.tm_year + 1900) * 10000 + (tm.tm_mon + 1) * 100 + tm.tm_mday;
    }

    inline std::vector<LotteryTicket> synthesize(std::size_t count, std::uint32_t seed = 42)
    {
        std::mt19937 rng(seed);
        static constexpr int   kNominals[] = { 50, 100, 150, 200, 500 };
        static constexpr int   kDaysRange = 365 * 4;
        static constexpr int   kWinProbPpm = 100;

        std::vector<LotteryTicket> out;
        out.reserve(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%08zu", i + 1);

            const int nominal = kNominals[rng() % 5];
            const int day = static_cast<int>(rng() % (kDaysRange + 1));
            const std::int32_t date = dateFromDayOffset(day);

            long long win = 0;
            if ((rng() % 1000) < kWinProbPpm)
            {
                switch (rng() % 5)
                {
                case 0:
                    win = nominal;
                    break;
                case 1:
                    win = static_cast<long long>(nominal) * 5;
                    break;
                case 2:
                    win = static_cast<long long>(nominal) * 10;
                    break;
                case 3:
                    win = static_cast<long long>(nominal) * 100;
                    break;
                case 4:
                    win = 100000;
                    break;
                }
            }

            out.emplace_back(std::string(buf), nominal, date, win);
        }
        return out;
    }
}
