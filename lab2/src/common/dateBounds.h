#pragma once

#include "lab2/common.h"

namespace lab2::detail {

inline constexpr auto firstDay = Day{std::chrono::year{2000} / 1 / 1};
inline constexpr auto lastDay = Day{std::chrono::year{2099} / 12 / 31};

} // namespace lab2::detail
