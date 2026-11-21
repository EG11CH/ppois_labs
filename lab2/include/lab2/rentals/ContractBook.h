#pragma once

#include "lab2/common.h"
#include "lab2/rentals/RentalContract.h"

#include <map>
#include <vector>

namespace lab2 {

class ContractBook {
  public:
    Id nextId() const;
    void store(RentalContract contract);
    const RentalContract& find(Id id) const;
    RentalContract& edit(Id id);
    int activeFor(Id customer) const;
    std::vector<Id> ids() const;

  private:
    std::map<Id, RentalContract> rows_;
    Id nextId_ = 1;
};

} // namespace lab2
