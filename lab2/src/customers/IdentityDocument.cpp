#include "lab2/customers/IdentityDocument.h"
#include "lab2/common.h"

namespace lab2 {

bool IdentityDocument::complete() const {
    return !number.empty() && !issuer.empty();
}

bool IdentityDocument::validOn(Day day) const {
    return complete() && day <= expires;
}

} // namespace lab2
