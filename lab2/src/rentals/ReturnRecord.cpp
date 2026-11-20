#include "lab2/rentals/ReturnRecord.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidContractState.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Branch.h"
#include "lab2/pricing/DamageAssessment.h"

namespace lab2 {

void ReturnRecord::verify(Id contract, const Asset& asset, const Branch& branch, Day issued) const {
    if (contractId != contract || branch.id != branchId || !branch.acceptsAt(asset.branch()) ||
        asset.state() != AssetState::rented || day < issued) {
        throw InvalidContractState("Проверь договор, пункт возврата и дату.");
    }
    inspection.verifyFor(asset.id(), day);
}

DamageAssessment ReturnRecord::assessment(const AssetModel& model) const {
    return {inspection.condition(), damageEstimate, model.replacementValue};
}

} // namespace lab2
