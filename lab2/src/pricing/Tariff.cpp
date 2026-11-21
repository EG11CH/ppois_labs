#include "lab2/pricing/Tariff.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidPayment.h"

#include <algorithm>

namespace lab2 {

namespace {
constexpr int daysPerWeek = 7;
} // namespace

void Tariff::validate() const {
    validateInputAmount(dailyRate);
    validateInputAmount(weeklyRate);
    if (dailyRate == 0 || weeklyRate < dailyRate || weeklyRate > dailyRate * daysPerWeek ||
        minimumDays < 1 || minimumDays > maxRentalDays) {
        throw InvalidPayment("Некорректные ставки или минимальный срок тарифа.");
    }
}

Money Tariff::rentalFor(const Period& period) const {
    validate();
    const auto billedDays = std::max(period.days(), minimumDays);
    const auto remainder = std::min((billedDays % daysPerWeek) * dailyRate, weeklyRate);
    return (billedDays / daysPerWeek) * weeklyRate + remainder;
}

} // namespace lab2
