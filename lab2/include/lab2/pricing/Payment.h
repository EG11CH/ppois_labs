#pragma once

#include "lab2/common.h"
#include "lab2/pricing/PaymentKind.h"

namespace lab2 {

struct Payment {
    Id contractId;
    PaymentKind kind;
    Money amount;
    Day day;
    bool matches(Id contract, PaymentKind expectedKind) const;
    void verify(Id contract, PaymentKind expectedKind, Money due, Day expectedDay) const;
    Money movement() const;
};

} // namespace lab2
