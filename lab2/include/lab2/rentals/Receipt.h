#pragma once

#include "lab2/common.h"
#include "lab2/pricing/Payment.h"
#include "lab2/pricing/Quote.h"

#include <string>

namespace lab2 {

struct Receipt {
    Id contractId;
    std::string clerkSignature;
    Day day;
    Payment payment;
    void verifyDeposit(const Quote& quote) const;
    std::string summary() const;
};

} // namespace lab2
