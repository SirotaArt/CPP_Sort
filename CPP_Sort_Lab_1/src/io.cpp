#include "io.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdio>

namespace io
{

    std::int32_t parseDate(const std::string& s)
    {
        if (s.size() != 10 || s[4] != '-' || s[7] != '-')
            throw std::runtime_error("bad date format: " + s);
        int y = std::stoi(s.substr(0, 4));
        int m = std::stoi(s.substr(5, 2));
        int d = std::stoi(s.substr(8, 2));
        return y * 10000 + m * 100 + d;
    }

    std::string formatDate(std::int32_t d)
    {
        int year = d / 10000;
        int month = (d / 100) % 100;
        int day = d % 100;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
        return std::string(buf);
    }

    static std::vector<std::string> split(const std::string& s, char sep = ',')
    {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s)
        {
            if (c == sep)
            {
                out.push_back(cur); cur.clear();
            }

            else if (c != '\r')
            {
                cur.push_back(c);
            }
        }

        out.push_back(cur);
        return out;
    }

    std::vector<LotteryTicket> readCsv(const std::string& path)
    {
        std::ifstream in(path);

        if (!in)
            throw std::runtime_error("cannot open: " + path);

        std::vector<LotteryTicket> out;
        std::string line;

        if (!std::getline(in, line)) return out;

        out.reserve(1 << 17);

        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto cols = split(line, ',');
            if (cols.size() < 4) continue;

            LotteryTicket t;
            t.ticketNumber = cols[0];
            t.nominalValue = std::stoi(cols[1]);
            t.lotteryDate = parseDate(cols[2]);
            t.winAmount = std::stoll(cols[3]);
            out.push_back(std::move(t));
        }
        return out;
    }

    void writeCsv(const std::string& path, const std::vector<LotteryTicket>& items) 
    {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("cannot write: " + path);
        out << "ticketNumber,nominalValue,lotteryDate,winAmount\n";

        for (const auto& t : items) 
        {
            out << t.ticketNumber << ','
                << t.nominalValue << ','
                << formatDate(t.lotteryDate) << ','
                << t.winAmount << '\n';
        }
    }
}
