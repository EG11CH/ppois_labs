#include "lab2/pricing.h"

#include <algorithm>

namespace lab2 {

namespace {
constexpr int daysPerWeek = 7;

void checkPercent(int percent) {
    if (percent < 0 || percent > maxPercent) {
        throw InvalidPayment("Процент вне диапазона 0–100.");
    }
}
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

int Discount::effectivePercent() const {
    checkPercent(membershipPercent);
    checkPercent(voucherPercent);
    checkPercent(capPercent);
    return std::min(std::max(membershipPercent, voucherPercent), capPercent);
}

Money Discount::saving(Money base) const {
    // Split the amount before multiplying so this also works for the full Money range.
    const auto percent = effectivePercent();
    return (base / maxPercent) * percent + (base % maxPercent) * percent / maxPercent;
}

bool Voucher::applies(const Period& period) const {
    return validity.covers(period) && period.days() >= minimumDays;
}

int Voucher::redeem(std::string_view submitted, const Period& period) const {
    if (submitted.empty()) {
        return 0;
    }
    if (submitted != code || !applies(period)) {
        throw InvalidPayment("Промокод не действует на выбранный срок.");
    }
    checkPercent(percent);
    return percent;
}

Money DepositPolicy::holdFor(const AssetModel& model) const {
    checkPercent(replacementPercent);
    validateInputAmount(minimum);
    validateInputAmount(maximum);
    validateInputAmount(model.replacementValue);
    if (minimum > maximum) {
        throw InvalidPayment("Минимальный залог превышает максимальный.");
    }
    const auto byValue = model.replacementValue * replacementPercent / maxPercent;
    return std::min(model.replacementValue, std::clamp(byValue, minimum, maximum));
}

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

Money Quote::rentalDue() const {
    return baseRent - discount;
}

Money Quote::dueAtPickup() const {
    return deposit;
}

bool Payment::matches(Id contract, PaymentKind expectedKind) const {
    return contractId == contract && kind == expectedKind;
}

void Payment::verify(Id contract, PaymentKind expectedKind, Money due, Day expectedDay) const {
    if (!matches(contract, expectedKind) || amount < 0 || amount != due || day != expectedDay) {
        throw InvalidPayment("Платёж должен совпасть с договором, суммой, видом и датой.");
    }
}

Money Payment::movement() const {
    return kind == PaymentKind::refund ? -amount : amount;
}

Money Settlement::totalCharges() const {
    return quote.rentalDue() + lateFee + damageFee;
}

Money Settlement::balanceDue() const {
    return std::max(Money{0}, totalCharges() - heldDeposit);
}

Money Settlement::refundDue() const {
    return std::max(Money{0}, heldDeposit - totalCharges());
}

} // namespace lab2
