#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/inventory/Branch.h"
#include "lab2/inventory/Inspection.h"
#include "lab2/pricing/DamageAssessment.h"

namespace lab2 {

struct ReturnRecord {
    Id contractId;
    Id branchId;
    Day day;
    Inspection inspection;
    Money damageEstimate;
    void verify(Id contract, const Asset& asset, const Branch& branch, Day issued) const;
    DamageAssessment assessment(const AssetModel& model) const;
};

} // namespace lab2
