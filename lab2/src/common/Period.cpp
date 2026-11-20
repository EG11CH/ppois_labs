#include "lab2/common/Period.h"
#include "dateBounds.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidPeriod.h"

#include <chrono>

namespace lab2 {

Period::Period(Day first, Day last) : start_{first}, end_{last} {
    if (first < detail::firstDay || last > detail::lastDay || first >= last ||
        last - first > std::chrono::days{maxRentalDays}) {
        throw InvalidPeriod("Нужен период от 1 до 365 дней в пределах 2000–2099 годов.");
    }
}

bool Period::contains(Day day) const {
    return start_ <= day && day < end_;
}

bool Period::overlaps(const Period& other) const {
    return start_ < other.end_ && other.start_ < end_;
}

bool Period::covers(const Period& other) const {
    return start_ <= other.start_ && other.end_ <= end_;
}

int Period::days() const {
    return static_cast<int>((end_ - start_).count());
}

Day Period::start() const {
    return start_;
}

Day Period::end() const {
    return end_;
}

} // namespace lab2
