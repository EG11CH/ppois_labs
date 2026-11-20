#include "lab2/maintenance/Part.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"

namespace lab2 {

void Part::validate() const {
    validateInputAmount(unitPrice);
    if (sku.empty() || name.empty() || supplier.empty()) {
        throw MaintenanceError("У детали должны быть артикул, название и поставщик.");
    }
}

Money Part::priceFor(int quantity) const {
    constexpr int maximumQuantity = 100;
    validate();
    if (quantity < 1 || quantity > maximumQuantity) {
        throw MaintenanceError("Некорректное количество деталей.");
    }
    return unitPrice * quantity;
}

} // namespace lab2
