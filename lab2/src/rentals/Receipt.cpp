#include "lab2/rentals/Receipt.h"
#include "lab2/common.h"
#include "lab2/pricing/PaymentKind.h"
#include "lab2/pricing/Quote.h"

#include <string>

namespace lab2 {

void Receipt::verifyDeposit(const Quote& quote) const {
    payment.verify(contractId, PaymentKind::deposit, quote.dueAtPickup(), day);
}

std::string Receipt::summary() const {
    return "Залог по договору #" + std::to_string(contractId) + ": " + formatMoney(payment.amount) +
           ", " + formatDay(day) + ", " + clerkSignature;
}

} // namespace lab2
