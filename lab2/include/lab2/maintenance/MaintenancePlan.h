#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Condition.h"
#include "lab2/maintenance/ServiceRule.h"

namespace lab2 {

struct MaintenancePlan {
    Id assetId;
    ServiceRule rule;
    Day lastService;
    int rentalCycles;
    bool dueOn(Day day) const;
    bool recordReturn(Day day, Condition condition);
    void reset(Day day);
};

} // namespace lab2
