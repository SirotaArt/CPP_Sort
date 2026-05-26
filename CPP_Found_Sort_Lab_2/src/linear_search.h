#pragma once

#include "lottery_ticket.h"
#include <string>
#include <vector>

namespace search
{

    inline std::vector<const LotteryTicket*>
        linearFindAll(const std::vector<LotteryTicket>& data, const std::string& key) 
    {
        std::vector<const LotteryTicket*> out;
        for (const auto& t : data)
            if (t.lotteryDate == key) out.push_back(&t);
        return out;
    }

}
