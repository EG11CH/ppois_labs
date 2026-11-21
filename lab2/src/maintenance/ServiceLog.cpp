#include "lab2/maintenance/ServiceLog.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/maintenance/MaintenanceJob.h"
#include "lab2/maintenance/RepairEstimate.h"

#include <optional>
#include <string>
#include <utility>

namespace lab2 {

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

} // namespace lab2
