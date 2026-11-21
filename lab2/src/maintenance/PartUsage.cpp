#include "lab2/maintenance/PartUsage.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"

#include <string>

namespace lab2 {

Money PartUsage::cost() const {
    if (reason.empty()) {
        throw MaintenanceError("Укажи причину замены детали.");
    }
    return part.priceFor(quantity);
}

std::string PartUsage::summary() const {
    return part.name + " × " + std::to_string(quantity) + ": " + formatMoney(cost());
}

} // namespace lab2
