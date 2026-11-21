#include "lab2/rentals/Incident.h"
#include "lab2/common.h"
#include "lab2/inventory/Condition.h"

#include <string>

namespace lab2 {

bool Incident::requiresService() const {
    return assessment.condition == Condition::damaged;
}

std::string Incident::summary() const {
    return "Предмет #" + std::to_string(assetId) + ", договор #" + std::to_string(contractId) +
           ", " + formatDay(day) + ": " + description;
}

} // namespace lab2
