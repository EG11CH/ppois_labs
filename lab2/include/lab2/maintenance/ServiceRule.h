#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Condition.h"

namespace lab2 {

struct ServiceRule {
    int maximumCycles;
    int maximumDays;
    bool inspectAfterDamage;
    bool due(int cycles, Day previous, Day today, Condition condition) const;
    void validate() const;
};

} // namespace lab2
