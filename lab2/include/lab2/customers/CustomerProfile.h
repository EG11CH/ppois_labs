#pragma once

#include "lab2/common.h"
#include "lab2/customers/Contact.h"
#include "lab2/customers/IdentityDocument.h"

#include <string>

namespace lab2 {

struct CustomerProfile {
    std::string name;
    Contact contact;
    IdentityDocument document;
    Day joined;
    void validate(Day asOf) const;
    bool canIdentify(Day day) const;
};

} // namespace lab2
