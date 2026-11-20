#pragma once

#include "lab2/common.h"

namespace lab2 {

struct LateFeePolicy {
    Money perDay;
    int graceDays;
    int maximumDays;
    int chargedDays(Day scheduled, Day returned) const;
    Money fee(Day scheduled, Day returned) const;
};

} // namespace lab2
