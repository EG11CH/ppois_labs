#include "lab2/rentals/ContractBook.h"
#include "lab2/common.h"
#include "lab2/errors/DuplicateId.h"
#include "lab2/errors/UnknownContract.h"
#include "lab2/rentals/RentalContract.h"

#include <utility>
#include <vector>

namespace lab2 {

Id ContractBook::nextId() const {
    return nextId_;
}

void ContractBook::store(RentalContract contract) {
    const auto id = contract.id();
    if (id != nextId_ || !rows_.try_emplace(id, std::move(contract)).second) {
        throw DuplicateId("Неверный номер нового договора.");
    }
    ++nextId_;
}

const RentalContract& ContractBook::find(Id id) const {
    const auto match = rows_.find(id);
    if (match == rows_.end()) {
        throw UnknownContract("Договор не найден.");
    }
    return match->second;
}

RentalContract& ContractBook::edit(Id id) {
    find(id);
    return rows_.at(id);
}

int ContractBook::activeFor(Id customer) const {
    int count = 0;
    for (const auto& [id, contract] : rows_) {
        count += contract.isActive() && contract.booking().customerId == customer;
    }
    return count;
}

std::vector<Id> ContractBook::ids() const {
    std::vector<Id> result;
    for (const auto& [id, contract] : rows_) {
        result.push_back(id);
    }
    return result;
}

} // namespace lab2
