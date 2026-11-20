#include "lab2/maintenance/MaintenancePlan.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/inventory/Condition.h"

namespace lab2 {

bool MaintenancePlan::dueOn(Day day) const {
    return rule.due(rentalCycles, lastService, day, Condition::good);
}

bool MaintenancePlan::recordReturn(Day day, Condition condition) {
    if (day < lastService) {
        throw MaintenanceError("Возврат датирован раньше последнего обслуживания.");
    }
    ++rentalCycles;
    return rule.due(rentalCycles, lastService, day, condition);
}

void MaintenancePlan::reset(Day day) {
    if (day < lastService) {
        throw MaintenanceError("Дата обслуживания не может идти назад.");
    }
    lastService = day;
    rentalCycles = 0;
}

} // namespace lab2
