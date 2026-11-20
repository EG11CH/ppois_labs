#include "lab2/pricing/Settlement.h"
#include "lab2/common.h"

#include <algorithm>

namespace lab2 {

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
