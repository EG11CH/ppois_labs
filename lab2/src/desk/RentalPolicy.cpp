#include "lab2/desk/RentalPolicy.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/inventory/Asset.h"
#include "lab2/pricing/Discount.h"
#include "lab2/pricing/Quote.h"
#include "lab2/pricing/Tariff.h"

namespace lab2 {

Quote RentalPolicy::price(const BookingRequest& request, const CustomerAccount& account,
                          const Asset& asset, const Tariff& tariff) const {
    const Discount discount{account.membership().discountOn(request.period.start()),
                            voucher.redeem(request.voucher, request.period), discountCap};
    const auto base = tariff.rentalFor(request.period);
    return {request.period, base, discount.saving(base), deposits.holdFor(asset.model())};
}

} // namespace lab2
