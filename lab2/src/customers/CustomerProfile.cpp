#include "lab2/customers/CustomerProfile.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidCustomer.h"

#include <string>

namespace lab2 {

void CustomerProfile::validate(Day asOf) const {
    if (name.find_first_not_of(" \t") == std::string::npos || !document.validOn(asOf) ||
        asOf < joined || contact.notificationTarget().empty()) {
        throw InvalidCustomer("Проверь имя, действующий документ, дату регистрации и контакты.");
    }
}

bool CustomerProfile::canIdentify(Day day) const {
    return day >= joined && document.validOn(day);
}

} // namespace lab2
