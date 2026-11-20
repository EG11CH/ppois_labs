#include "lab2/customers/Contact.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace lab2 {

bool Contact::canReceiveEmail() const {
    const auto separator = email.find('@');
    return acceptsEmail && separator != std::string::npos && separator > 0 &&
           separator + 1 < email.size() && email.find('@', separator + 1) == std::string::npos &&
           email.find_first_of(" \t\r\n") == std::string::npos;
}

bool Contact::hasPhone() const {
    constexpr std::size_t minDigits = 7;
    constexpr std::size_t maxDigits = 15;
    const auto digits = phone.starts_with('+') ? std::string_view{phone}.substr(1) : phone;
    return digits.size() >= minDigits && digits.size() <= maxDigits &&
           digits.find_first_not_of("0123456789") == std::string_view::npos;
}

std::string Contact::notificationTarget() const {
    if (canReceiveEmail()) {
        return email;
    }
    return hasPhone() ? phone : "";
}

} // namespace lab2
