#include "lab2/inventory/InspectionChecklist.h"

namespace lab2 {

bool InspectionChecklist::passed() const {
    return defects() == 0;
}

int InspectionChecklist::defects() const {
    return static_cast<int>(!powersOn) + static_cast<int>(!casingIntact) +
           static_cast<int>(!accessoriesComplete);
}

} // namespace lab2
