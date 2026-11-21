#include "lab2/inventory/Inventory.h"
#include "lab2/common.h"
#include "lab2/errors/DuplicateId.h"
#include "lab2/errors/InvalidInventory.h"
#include "lab2/errors/UnknownAsset.h"
#include "lab2/errors/UnknownBranch.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Branch.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace lab2 {

void Inventory::addBranch(Branch branch) {
    if (branch.id <= 0 || branch.name.empty() || branch.address.empty()) {
        throw InvalidInventory("Проверь номер, название и адрес пункта.");
    }
    const auto id = branch.id;
    if (!branches_.try_emplace(id, std::move(branch)).second) {
        throw DuplicateId("Пункт с таким номером уже есть.");
    }
}

void Inventory::addAsset(Asset asset) {
    const auto& office = branch(asset.branch());
    if (!office.acceptsAt(asset.branch())) {
        throw InvalidInventory("Этот пункт не принимает возвраты и не может выдавать инвентарь.");
    }
    const auto sameSerial = [&asset](const auto& entry) {
        return entry.second.serial() == asset.serial();
    };
    if (std::ranges::any_of(assets_, sameSerial)) {
        throw DuplicateId("Серийный номер предмета уже используется.");
    }
    const auto id = asset.id();
    if (!assets_.try_emplace(id, std::move(asset)).second) {
        throw DuplicateId("Предмет с таким номером уже есть.");
    }
}

const Branch& Inventory::branch(Id id) const {
    const auto match = branches_.find(id);
    if (match == branches_.end()) {
        throw UnknownBranch("Пункт проката не найден.");
    }
    return match->second;
}

const Asset& Inventory::find(Id id) const {
    const auto match = assets_.find(id);
    if (match == assets_.end()) {
        throw UnknownAsset("Предмет не найден.");
    }
    return match->second;
}

Asset& Inventory::edit(Id id) {
    find(id);
    return assets_.at(id);
}

std::vector<Id> Inventory::readyAt(Id branchId) const {
    branch(branchId);
    std::vector<Id> result;
    for (const auto& [id, asset] : assets_) {
        if (asset.branch() == branchId && asset.canIssue()) {
            result.push_back(id);
        }
    }
    return result;
}

int Inventory::count(AssetState state) const {
    int total = 0;
    for (const auto& [id, asset] : assets_) {
        total += asset.state() == state;
    }
    return total;
}

std::vector<Id> Inventory::assetIds() const {
    std::vector<Id> result;
    for (const auto& [id, asset] : assets_) {
        result.push_back(id);
    }
    return result;
}

} // namespace lab2
