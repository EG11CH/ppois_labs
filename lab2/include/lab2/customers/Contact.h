#pragma once

#include <string>

namespace lab2 {

struct Contact {
    std::string phone;
    std::string email;
    bool acceptsEmail;
    bool canReceiveEmail() const;
    bool hasPhone() const;
    std::string notificationTarget() const;
};

} // namespace lab2
