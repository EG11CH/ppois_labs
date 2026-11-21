#pragma once

#include "lab2/common.h"
#include "lab2/pricing/Payment.h"
#include "lab2/pricing/Settlement.h"
#include "lab2/rentals/Receipt.h"

#include <string>

namespace lab2 {

struct Invoice {
    Id contractId;
    Day issuedOn;
    Settlement settlement;
    Payment finalPayment;
    void verify() const;
    Money netCollected(const Receipt& receipt) const;
    std::string summary() const;
};

} // namespace lab2
