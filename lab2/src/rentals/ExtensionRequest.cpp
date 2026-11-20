#include "lab2/rentals/ExtensionRequest.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidContractState.h"

namespace lab2 {

Period ExtensionRequest::applyTo(Id contract, Day issued, const Period& current) const {
    if (contractId != contract || requestedOn < issued || requestedOn > current.end() ||
        until <= current.end() || reason.empty()) {
        throw InvalidContractState(
            "Продление нужно оформить до срока возврата с указанием причины.");
    }
    return {current.start(), until};
}

} // namespace lab2
