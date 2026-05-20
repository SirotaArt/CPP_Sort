#pragma once
#include "lottery_ticket.h"
#include <vector>
#include <string>

namespace io
{
	std::vector<LotteryTicket> readCsv(const std::string& path);

	void writeCsv(const std::string& path, const std::vector<LotteryTicket>& items);

	std::int32_t parseDate(const std::string& s);

	std::string formatDate(std::int32_t d);

}
