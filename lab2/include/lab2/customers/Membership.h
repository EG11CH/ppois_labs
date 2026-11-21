#pragma once

#include "lab2/common.h"

#include <string>

namespace lab2 {

struct Membership {
    std::string name;
    int discountPercent;
    Day validUntil;
    int discountOn(Day day) const;
    void renew(Day newDeadline);
};

} // namespace lab2
