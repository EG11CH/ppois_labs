#pragma once

#include "lab2/common.h"
#include "lab2/pricing/Quote.h"

namespace lab2 {

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
