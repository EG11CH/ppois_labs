#include "lab2/pricing/Payment.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidPayment.h"
#include "lab2/pricing/PaymentKind.h"

namespace lab2 {

bool Payment::matches(Id contract, PaymentKind expectedKind) const {
    return contractId == contract && kind == expectedKind;
}

void Payment::verify(Id contract, PaymentKind expectedKind, Money due, Day expectedDay) const {
    if (!matches(contract, expectedKind) || amount < 0 || amount != due || day != expectedDay) {
        throw InvalidPayment("Платёж должен совпасть с договором, суммой, видом и датой.");
    }
}

Money Payment::movement() const {
    return kind == PaymentKind::refund ? -amount : amount;
}

} // namespace lab2
