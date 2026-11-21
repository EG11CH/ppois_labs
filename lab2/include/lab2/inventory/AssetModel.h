#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Category.h"

#include <string>

namespace lab2 {

struct AssetModel {
    std::string name;
    std::string manufacturer;
    Category category;
    Money replacementValue;
    void validate() const;
    std::string label() const;
};

} // namespace lab2
