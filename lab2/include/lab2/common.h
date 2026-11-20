#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace lab2 {

using Day = std::chrono::sys_days;
using Id = int;
using Money = std::int64_t; // копейки
inline constexpr Money maxInputAmount = 100'000'000;
inline constexpr int maxRentalDays = 365;
inline constexpr int maxPercent = 100;

Day parseDay(std::string_view text);
std::string formatDay(Day day);
std::string formatMoney(Money amount);
void validateInputAmount(Money amount);

} // namespace lab2
