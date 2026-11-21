#include "lab2/pricing/LateFeePolicy.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidPayment.h"

#include <algorithm>
#include <chrono>

namespace lab2 {

int LateFeePolicy::chargedDays(Day scheduled, Day returned) const {
    if (graceDays < 0 || maximumDays < 0) {
        throw InvalidPayment("Льгота и предел просрочки задаются неотрицательными днями.");
    }
    using Count = std::chrono::days::rep;
    const auto days = (returned - scheduled).count() - graceDays;
    return static_cast<int>(std::clamp<Count>(days, 0, maximumDays));
}

Money LateFeePolicy::fee(Day scheduled, Day returned) const {
    validateInputAmount(perDay);
    return perDay * chargedDays(scheduled, returned);
}

} // namespace lab2
