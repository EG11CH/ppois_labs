#pragma once

#include "lab2/inventory.h"

namespace lab2 {

struct Tariff {
    Money dailyRate;
    Money weeklyRate;
    int minimumDays;
    void validate() const;
    Money rentalFor(const Period& period) const;
};

struct Discount {
    int membershipPercent;
    int voucherPercent;
    int capPercent;
    int effectivePercent() const;
    Money saving(Money base) const;
};

struct Voucher {
    std::string code;
    Period validity;
    int percent;
    int minimumDays;
    bool applies(const Period& period) const;
    int redeem(std::string_view submitted, const Period& period) const;
};

struct DepositPolicy {
    int replacementPercent;
    Money minimum;
    Money maximum;
    Money holdFor(const AssetModel& model) const;
};

struct LateFeePolicy {
    Money perDay;
    int graceDays;
    int maximumDays;
    int chargedDays(Day scheduled, Day returned) const;
    Money fee(Day scheduled, Day returned) const;
};

struct DamageAssessment {
    Condition condition;
    Money estimatedRepair;
    Money replacementValue;
    void validate() const;
    Money charge() const;
    Money uncoveredLoss() const;
};

struct Quote {
    Period period;
    Money baseRent;
    Money discount;
    Money deposit;
    Money rentalDue() const;
    Money dueAtPickup() const;
};

enum class PaymentKind { deposit, balance, refund };

struct Payment {
    Id contractId;
    PaymentKind kind;
    Money amount;
    Day day;
    bool matches(Id contract, PaymentKind expectedKind) const;
    void verify(Id contract, PaymentKind expectedKind, Money due, Day expectedDay) const;
    Money movement() const;
};

struct Settlement {
    Quote quote;
    Money lateFee;
    Money damageFee;
    Money heldDeposit;
    Money totalCharges() const;
    Money balanceDue() const;
    Money refundDue() const;
};

} // namespace lab2
