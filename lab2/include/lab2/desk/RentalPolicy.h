#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/HoldPolicy.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/customers/Eligibility.h"
#include "lab2/inventory/Asset.h"
#include "lab2/maintenance/ServiceRule.h"
#include "lab2/pricing/DepositPolicy.h"
#include "lab2/pricing/LateFeePolicy.h"
#include "lab2/pricing/Quote.h"
#include "lab2/pricing/Tariff.h"
#include "lab2/pricing/Voucher.h"

namespace lab2 {

struct RentalPolicy {
    Eligibility eligibility;
    HoldPolicy holds;
    DepositPolicy deposits;
    LateFeePolicy lateFees;
    Voucher voucher;
    ServiceRule service;
    int discountCap;
    Quote price(const BookingRequest& request, const CustomerAccount& account, const Asset& asset,
                const Tariff& tariff) const;
};

} // namespace lab2
