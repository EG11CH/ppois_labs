#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/inventory/Asset.h"
#include "lab2/maintenance/MaintenancePlan.h"
#include "lab2/pricing/Tariff.h"

namespace lab2 {

struct AssetTerms {
    Tariff tariff;
    MaintenancePlan maintenance;
    Day lastMovement;
    bool readyFor(const Asset& asset, const Period& period) const;
    void verifyTimeline(Day day) const;
};

} // namespace lab2
