#include "lab2/maintenance/RepairEstimate.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"

namespace lab2 {

Money RepairEstimate::total() const {
    validateInputAmount(labor);
    Money amount = labor;
    // Estimates are bounded even if a caller submits many part entries.
    for (const auto& entry : parts) {
        amount += entry.cost();
        if (amount > maxInputAmount) {
            throw MaintenanceError("Общая стоимость ремонта вне допустимого диапазона.");
        }
    }
    return amount;
}

void RepairEstimate::requireWithinBudget() const {
    validateInputAmount(limit);
    if (total() > limit) {
        throw MaintenanceError("Ремонт превышает согласованный бюджет.");
    }
}

} // namespace lab2
