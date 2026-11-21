#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"

namespace lab2 {

struct Quote {
    Period period;
    Money baseRent;
    Money discount;
    Money deposit;
    Money rentalDue() const;
    Money dueAtPickup() const;
};

} // namespace lab2
