#include "lab2/maintenance/MaintenanceDesk.h"
#include "lab2/common.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/maintenance/MaintenancePlan.h"
#include "lab2/maintenance/RepairEstimate.h"
#include "lab2/maintenance/ServiceLog.h"
#include "lab2/maintenance/ServiceRule.h"

#include <string>
#include <utility>

namespace lab2 {

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
