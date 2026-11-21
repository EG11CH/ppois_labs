#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Condition.h"

namespace lab2 {

struct DamageAssessment {
    Condition condition;
    Money estimatedRepair;
    Money replacementValue;
    void validate() const;
    Money charge() const;
    Money uncoveredLoss() const;
};

} // namespace lab2
