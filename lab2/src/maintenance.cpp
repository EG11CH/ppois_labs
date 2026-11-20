#include "lab2/maintenance.h"

#include <utility>

namespace lab2 {

bool ServiceRule::due(int cycles, Day previous, Day today, Condition condition) const {
    return cycles >= maximumCycles || today - previous >= std::chrono::days{maximumDays} ||
           (inspectAfterDamage && condition == Condition::damaged);
}

void ServiceRule::validate() const {
    if (maximumCycles < 1 || maximumDays < 1) {
        throw MaintenanceError("Периодичность обслуживания должна быть положительной.");
    }
}

bool MaintenancePlan::dueOn(Day day) const {
    return rule.due(rentalCycles, lastService, day, Condition::good);
}

bool MaintenancePlan::recordReturn(Day day, Condition condition) {
    if (day < lastService) {
        throw MaintenanceError("Возврат датирован раньше последнего обслуживания.");
    }
    ++rentalCycles;
    return rule.due(rentalCycles, lastService, day, condition);
}

void MaintenancePlan::reset(Day day) {
    if (day < lastService) {
        throw MaintenanceError("Дата обслуживания не может идти назад.");
    }
    lastService = day;
    rentalCycles = 0;
}

void Part::validate() const {
    validateInputAmount(unitPrice);
    if (sku.empty() || name.empty() || supplier.empty()) {
        throw MaintenanceError("У детали должны быть артикул, название и поставщик.");
    }
}

Money Part::priceFor(int quantity) const {
    constexpr int maximumQuantity = 100;
    validate();
    if (quantity < 1 || quantity > maximumQuantity) {
        throw MaintenanceError("Некорректное количество деталей.");
    }
    return unitPrice * quantity;
}

Money PartUsage::cost() const {
    if (reason.empty()) {
        throw MaintenanceError("Укажи причину замены детали.");
    }
    return part.priceFor(quantity);
}

std::string PartUsage::summary() const {
    return part.name + " × " + std::to_string(quantity) + ": " + formatMoney(cost());
}

Money RepairEstimate::total() const {
    validateInputAmount(labor);
    Money amount = labor;
    // Estimates are bounded even if a caller submits many part entries.
    for (const auto& entry : parts) {
        amount += entry.cost();
        if (amount > maxInputAmount) {
            throw MaintenanceError("Общая стоимость ремонта вне допустимого диапазона.");
        }
    }
    return amount;
}

void RepairEstimate::requireWithinBudget() const {
    validateInputAmount(limit);
    if (total() > limit) {
        throw MaintenanceError("Ремонт превышает согласованный бюджет.");
    }
}

MaintenanceJob::MaintenanceJob(Id id, Id assetId, Day opened, std::string reason)
    : id_{id}, assetId_{assetId}, opened_{opened}, reason_{std::move(reason)} {
    if (reason_.empty()) {
        throw MaintenanceError("Укажи причину обслуживания.");
    }
}

Id MaintenanceJob::id() const {
    return id_;
}

Id MaintenanceJob::assetId() const {
    return assetId_;
}

bool MaintenanceJob::open() const {
    return !closed_;
}

void MaintenanceJob::finish(Day day, RepairEstimate estimate) {
    if (!open() || day < opened_) {
        throw MaintenanceError("Заказ закрыт или дата завершения предшествует открытию.");
    }
    estimate.requireWithinBudget();
    estimate_.emplace(std::move(estimate));
    closed_ = day;
}

Money MaintenanceJob::expense() const {
    return estimate_ ? estimate_->total() : 0;
}

std::string MaintenanceJob::summary() const {
    const auto status = closed_ ? "закрыт " + formatDay(*closed_) : "открыт";
    return "Заказ #" + std::to_string(id_) + ", предмет #" + std::to_string(assetId_) + ", " +
           formatDay(opened_) + ", " + status + ": " + reason_;
}

Id ServiceLog::openFor(Id asset, Day day, std::string reason) {
    if (activeFor(asset)) {
        throw MaintenanceError("Для предмета уже открыт заказ на обслуживание.");
    }
    const auto id = nextId_;
    jobs_.try_emplace(id, id, asset, day, std::move(reason));
    ++nextId_;
    return id;
}

const MaintenanceJob& ServiceLog::find(Id id) const {
    const auto match = jobs_.find(id);
    if (match == jobs_.end()) {
        throw MaintenanceError("Заказ на обслуживание не найден.");
    }
    return match->second;
}

void ServiceLog::close(Id id, Day day, RepairEstimate estimate) {
    find(id);
    jobs_.at(id).finish(day, std::move(estimate));
}

std::optional<Id> ServiceLog::activeFor(Id asset) const {
    for (const auto& [id, job] : jobs_) {
        if (job.assetId() == asset && job.open()) {
            return id;
        }
    }
    return std::nullopt;
}

Money ServiceLog::totalFor(Id asset) const {
    Money amount = 0;
    for (const auto& [id, job] : jobs_) {
        if (job.assetId() == asset) {
            amount += job.expense();
        }
    }
    return amount;
}

MaintenanceDesk::MaintenanceDesk(ServiceRule rule) : rule_{rule} {
    rule_.validate();
}

MaintenancePlan MaintenanceDesk::planFor(Id asset, Day commissioned) const {
    return {asset, rule_, commissioned, 0};
}

Id MaintenanceDesk::start(Asset& asset, Day day, std::string reason) {
    if (asset.state() != AssetState::ready && asset.state() != AssetState::service) {
        throw MaintenanceError("Предмет должен быть на складе или в ремонте.");
    }
    const auto id = log_.openFor(asset.id(), day, std::move(reason));
    asset.sendToService();
    return id;
}

void MaintenanceDesk::finish(Asset& asset, MaintenancePlan& plan, Id job, Day day,
                             RepairEstimate estimate) {
    if (asset.state() != AssetState::service || plan.assetId != asset.id() ||
        log_.find(job).assetId() != asset.id()) {
        throw MaintenanceError("Предмет, план и заказ должны относиться к одному ремонту.");
    }
    auto updatedPlan = plan;
    updatedPlan.reset(day);
    log_.close(job, day, std::move(estimate));
    plan = updatedPlan;
    asset.releaseFromService();
}

const ServiceLog& MaintenanceDesk::log() const {
    return log_;
}

} // namespace lab2
