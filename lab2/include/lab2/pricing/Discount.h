#pragma once

#include "lab2/common.h"

namespace lab2 {

struct Discount {
    int membershipPercent;
    int voucherPercent;
    int capPercent;
    int effectivePercent() const;
    Money saving(Money base) const;
};

} // namespace lab2
