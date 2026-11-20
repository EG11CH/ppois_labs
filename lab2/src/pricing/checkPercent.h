#pragma once

#include "lab2/common.h"
#include "lab2/errors/InvalidPayment.h"

namespace lab2::detail {

inline void checkPercent(int percent) {
    if (percent < 0 || percent > maxPercent) {
        throw InvalidPayment("Процент вне диапазона 0–100.");
    }
}

} // namespace lab2::detail
