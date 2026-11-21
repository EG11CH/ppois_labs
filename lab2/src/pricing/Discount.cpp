#include "lab2/pricing/Discount.h"
#include "checkPercent.h"
#include "lab2/common.h"

#include <algorithm>

namespace lab2 {

int Discount::effectivePercent() const {
    detail::checkPercent(membershipPercent);
    detail::checkPercent(voucherPercent);
    detail::checkPercent(capPercent);
    return std::min(std::max(membershipPercent, voucherPercent), capPercent);
}

Money Discount::saving(Money base) const {
    // Split the amount before multiplying so this also works for the full Money range.
    const auto percent = effectivePercent();
    return (base / maxPercent) * percent + (base % maxPercent) * percent / maxPercent;
}

} // namespace lab2
