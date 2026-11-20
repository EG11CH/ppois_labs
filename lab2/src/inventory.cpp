#include "lab2/inventory.h"

#include <algorithm>
#include <utility>

namespace lab2 {

bool Branch::acceptsAt(Id assetBranch) const {
    return acceptsReturns && id == assetBranch;
}

std::string Branch::label() const {
    return name + " — " + address;
}

void Category::validate() const {
    if (code.empty() || name.empty()) {
        throw InvalidInventory("У категории нужны код и название.");
    }
}

bool Category::permitsIssue(bool briefingGiven) const {
    return !requiresBriefing || briefingGiven;
}

void AssetModel::validate() const {
    category.validate();
    validateInputAmount(replacementValue);
    if (name.empty() || manufacturer.empty() || replacementValue == 0) {
        throw InvalidInventory("У модели нужны имя, производитель и цена замены.");
    }
}

std::string AssetModel::label() const {
    return manufacturer + " " + name;
}

Asset::Asset(Id id, Id branch, std::string serial, AssetModel model)
    : id_{id}, branch_{branch}, serial_{std::move(serial)}, model_{std::move(model)} {
    model_.validate();
    if (id_ <= 0 || branch_ <= 0 || serial_.empty()) {
        throw InvalidInventory("Нужны положительные номера и серийный номер предмета.");
    }
}

Id Asset::id() const {
    return id_;
}

Id Asset::branch() const {
    return branch_;
}

const std::string& Asset::serial() const {
    return serial_;
}

const AssetModel& Asset::model() const {
    return model_;
}

AssetState Asset::state() const {
    return state_;
}

bool Asset::canIssue() const {
    return state_ == AssetState::ready;
}

void Asset::issue() {
    if (!canIssue()) {
        throw AssetUnavailable("Предмет сейчас нельзя выдать.");
    }
    state_ = AssetState::rented;
}

void Asset::receive(Condition condition) {
    if (state_ != AssetState::rented) {
        throw InvalidContractState("Предмет должен быть выдан, чтобы принять его обратно.");
    }
    state_ = condition == Condition::good ? AssetState::ready : AssetState::service;
}

void Asset::sendToService() {
    if (state_ != AssetState::ready && state_ != AssetState::service) {
        throw MaintenanceError("Этот предмет сейчас нельзя отправить в ремонт.");
    }
    state_ = AssetState::service;
}

void Asset::releaseFromService() {
    if (state_ != AssetState::service) {
        throw MaintenanceError("Предмет не находится в ремонте.");
    }
    state_ = AssetState::ready;
}

void Asset::retire() {
    if (state_ != AssetState::ready) {
        throw InvalidInventory("Списать можно только готовый к выдаче предмет.");
    }
    state_ = AssetState::retired;
}

bool InspectionChecklist::passed() const {
    return defects() == 0;
}

int InspectionChecklist::defects() const {
    return static_cast<int>(!powersOn) + static_cast<int>(!casingIntact) +
           static_cast<int>(!accessoriesComplete);
}

Condition Inspection::condition() const {
    return checklist.passed() ? Condition::good : Condition::damaged;
}

void Inspection::verifyFor(Id asset, Day expectedDay) const {
    if (asset != assetId || expectedDay != day || (!checklist.passed() && note.empty())) {
        throw InvalidInspection("Проверь номер, дату осмотра и описание дефектов.");
    }
}

void Inspection::verifyBeforeIssue(Id asset, Day expectedDay) const {
    verifyFor(asset, expectedDay);
    if (condition() == Condition::damaged) {
        throw InvalidInspection("Выдача требует осмотра без дефектов.");
    }
}

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

bool Availability::canReserve() const {
    return physicallyReady && !booked;
}

void Availability::requireAvailable() const {
    if (!physicallyReady) {
        throw AssetUnavailable("Предмет пока недоступен.");
    }
    if (booked) {
        throw BookingConflict("Этот период пересекается с действующей бронью.");
    }
}

} // namespace lab2
