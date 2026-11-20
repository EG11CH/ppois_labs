#pragma once

#include "lab2/common.h"
#include "lab2/inventory/AssetModel.h"

namespace lab2 {

struct DepositPolicy {
    int replacementPercent;
    Money minimum;
    Money maximum;
    Money holdFor(const AssetModel& model) const;
};

} // namespace lab2
