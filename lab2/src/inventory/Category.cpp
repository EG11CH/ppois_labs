#include "lab2/inventory/Category.h"
#include "lab2/errors/InvalidInventory.h"

namespace lab2 {

void Category::validate() const {
    if (code.empty() || name.empty()) {
        throw InvalidInventory("У категории нужны код и название.");
    }
}

bool Category::permitsIssue(bool briefingGiven) const {
    return !requiresBriefing || briefingGiven;
}

} // namespace lab2
