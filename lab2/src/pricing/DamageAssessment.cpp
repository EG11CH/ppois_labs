#include "lab2/pricing/DamageAssessment.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidInspection.h"
#include "lab2/inventory/Condition.h"

#include <algorithm>

namespace lab2 {

void DamageAssessment::validate() const {
    validateInputAmount(estimatedRepair);
    validateInputAmount(replacementValue);
    if (condition == Condition::good && estimatedRepair != 0) {
        throw InvalidInspection("Целый предмет нельзя оценить как повреждённый.");
    }
}

Money DamageAssessment::charge() const {
    validate();
    return std::min(estimatedRepair, replacementValue);
}

Money DamageAssessment::uncoveredLoss() const {
    return estimatedRepair - charge();
}

} // namespace lab2
