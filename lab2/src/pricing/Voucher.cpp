#include "lab2/pricing/Voucher.h"
#include "checkPercent.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidPayment.h"

#include <string_view>

namespace lab2 {

bool Voucher::applies(const Period& period) const {
    return validity.covers(period) && period.days() >= minimumDays;
}

int Voucher::redeem(std::string_view submitted, const Period& period) const {
    if (submitted.empty()) {
        return 0;
    }
    if (submitted != code || !applies(period)) {
        throw InvalidPayment("Промокод не действует на выбранный срок.");
    }
    detail::checkPercent(percent);
    return percent;
}

} // namespace lab2
