#pragma once

#include "lottery_ticket.h"
#include <vector>
#include <string>

namespace io
{
    std::vector<LotteryTicket> readCsv(const std::string& path);
    void writeFound(const std::string& path, const std::string& method, const std::string& key, const std::vector<const LotteryTicket*>& results);
}
