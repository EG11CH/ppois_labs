#include "lab2/pricing/DepositPolicy.h"
#include "checkPercent.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidPayment.h"
#include "lab2/inventory/AssetModel.h"

#include <algorithm>

namespace lab2 {

Money DepositPolicy::holdFor(const AssetModel& model) const {
    detail::checkPercent(replacementPercent);
    validateInputAmount(minimum);
    validateInputAmount(maximum);
    validateInputAmount(model.replacementValue);
    if (minimum > maximum) {
        throw InvalidPayment("Минимальный залог превышает максимальный.");
    }
    const auto byValue = model.replacementValue * replacementPercent / maxPercent;
    return std::min(model.replacementValue, std::clamp(byValue, minimum, maximum));
}

} // namespace lab2
