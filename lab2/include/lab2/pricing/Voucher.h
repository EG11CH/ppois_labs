#pragma once

#include "lab2/common/Period.h"

#include <string>
#include <string_view>

namespace lab2 {

struct Voucher {
    std::string code;
    Period validity;
    int percent;
    int minimumDays;
    bool applies(const Period& period) const;
    int redeem(std::string_view submitted, const Period& period) const;
};

} // namespace lab2
