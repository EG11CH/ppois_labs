#include "lab2/inventory/AssetModel.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidInventory.h"

#include <string>

namespace lab2 {

void AssetModel::validate() const {
    category.validate();
    validateInputAmount(replacementValue);
    if (name.empty() || manufacturer.empty() || replacementValue == 0) {
        throw InvalidInventory("У модели нужны имя, производитель и цена замены.");
    }
}

std::string AssetModel::label() const {
    return manufacturer + " " + name;
}

} // namespace lab2
