#include "lab2/inventory/Asset.h"
#include "lab2/common.h"
#include "lab2/errors/AssetUnavailable.h"
#include "lab2/errors/InvalidContractState.h"
#include "lab2/errors/InvalidInventory.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Condition.h"

#include <string>
#include <utility>

namespace lab2 {

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

} // namespace lab2
