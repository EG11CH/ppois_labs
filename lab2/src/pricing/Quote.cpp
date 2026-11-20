#include "lab2/pricing/Quote.h"
#include "lab2/common.h"

namespace lab2 {

Money Quote::rentalDue() const {
    return baseRent - discount;
}

Money Quote::dueAtPickup() const {
    return deposit;
}

} // namespace lab2
