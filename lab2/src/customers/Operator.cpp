#include "lab2/customers/Operator.h"
#include "lab2/common.h"

#include <string>

namespace lab2 {

bool Operator::mayHandle(Id branch) const {
    return id > 0 && branchId == branch && !name.empty();
}

std::string Operator::signature() const {
    return name + " #" + std::to_string(id);
}

} // namespace lab2
