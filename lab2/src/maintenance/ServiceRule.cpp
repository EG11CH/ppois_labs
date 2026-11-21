#include "lab2/maintenance/ServiceRule.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/inventory/Condition.h"

#include <chrono>

namespace lab2 {

bool ServiceRule::due(int cycles, Day previous, Day today, Condition condition) const {
    return cycles >= maximumCycles || today - previous >= std::chrono::days{maximumDays} ||
           (inspectAfterDamage && condition == Condition::damaged);
}

void ServiceRule::validate() const {
    if (maximumCycles < 1 || maximumDays < 1) {
        throw MaintenanceError("Периодичность обслуживания должна быть положительной.");
    }
}

} // namespace lab2
