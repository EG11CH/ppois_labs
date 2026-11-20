#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Condition.h"
#include "lab2/inventory/InspectionChecklist.h"

#include <string>

namespace lab2 {

struct Inspection {
    Id assetId;
    Day day;
    InspectionChecklist checklist;
    std::string note;
    Condition condition() const;
    void verifyFor(Id asset, Day expectedDay) const;
    void verifyBeforeIssue(Id asset, Day expectedDay) const;
};

} // namespace lab2
