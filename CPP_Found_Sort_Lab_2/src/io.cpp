#include "io.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace io
{

    static std::vector<std::string> split(const std::string& s, char sep = ',')
    {
        std::vector<std::string> out; std::string cur;
        for (char c : s)
        {
            if (c == sep) { out.push_back(cur); cur.clear(); }
            else if (c != '\r') cur.push_back(c);
        }
        out.push_back(cur);
        return out;
    }

    std::vector<LotteryTicket> readCsv(const std::string& path)
    {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open: " + path);
        std::vector<LotteryTicket> out; out.reserve(1 << 17);
        std::string line;
        if (!std::getline(in, line)) return out;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto c = split(line, ',');
            if (c.size() < 4) continue;
            out.emplace_back(c[0], std::stoi(c[1]), c[2], std::stoll(c[3]));
        }
        return out;
    }

    void writeFound(const std::string& path, const std::string& method, const std::string& key, const std::vector<const LotteryTicket*>& results)
    {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("cannot write: " + path);
        out << "# method=" << method << " key=" << key
            << " matches=" << results.size() << "\n";
        out << "ticketNumber,nominalValue,lotteryDate,winAmount\n";
        for (const auto* t : results) {
            out << t->ticketNumber << ','
                << t->nominalValue << ','
                << t->lotteryDate << ','
                << t->winAmount << '\n';
        }
    }

}
