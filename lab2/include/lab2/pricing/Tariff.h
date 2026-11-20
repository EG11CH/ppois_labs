#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"

namespace lab2 {

struct Tariff {
    Money dailyRate;
    Money weeklyRate;
    int minimumDays;
    void validate() const;
    Money rentalFor(const Period& period) const;
};

} // namespace lab2
