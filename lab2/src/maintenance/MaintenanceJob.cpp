#include "lab2/maintenance/MaintenanceJob.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/maintenance/RepairEstimate.h"

#include <string>
#include <utility>

namespace lab2 {

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

} // namespace lab2
