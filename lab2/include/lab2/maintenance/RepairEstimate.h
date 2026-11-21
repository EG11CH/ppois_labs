#pragma once

#include "lab2/common.h"
#include "lab2/maintenance/PartUsage.h"

#include <vector>

namespace lab2 {

struct RepairEstimate {
    Money labor;
    std::vector<PartUsage> parts;
    Money limit;
    Money total() const;
    void requireWithinBudget() const;
};

} // namespace lab2
