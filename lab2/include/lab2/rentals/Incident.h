#pragma once

#include "lab2/common.h"
#include "lab2/pricing/DamageAssessment.h"

#include <string>

namespace lab2 {

struct Incident {
    Id contractId;
    Id assetId;
    Day day;
    std::string description;
    DamageAssessment assessment;
    bool requiresService() const;
    std::string summary() const;
};

} // namespace lab2
