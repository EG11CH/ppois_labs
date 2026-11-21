#include "lab2/desk/AssetTerms.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidPeriod.h"
#include "lab2/inventory/Asset.h"

namespace lab2 {

bool AssetTerms::readyFor(const Asset& asset, const Period& period) const {
    return asset.canIssue() && lastOperation <= period.start() &&
           !maintenance.dueOn(period.start());
}

void AssetTerms::verifyTimeline(Day day) const {
    if (day < lastOperation) {
        throw InvalidPeriod("Дата раньше предыдущей операции с предметом.");
    }
}

} // namespace lab2
