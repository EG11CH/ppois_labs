#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Branch.h"

#include <map>
#include <vector>

namespace lab2 {

class Inventory {
  public:
    void addBranch(Branch branch);
    void addAsset(Asset asset);
    const Branch& branch(Id id) const;
    const Asset& find(Id id) const;
    Asset& edit(Id id);
    std::vector<Id> readyAt(Id branchId) const;
    int count(AssetState state) const;
    std::vector<Id> assetIds() const;

  private:
    std::map<Id, Branch> branches_;
    std::map<Id, Asset> assets_;
};

} // namespace lab2
