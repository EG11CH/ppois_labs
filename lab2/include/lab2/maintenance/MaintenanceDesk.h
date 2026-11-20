#pragma once

#include "lab2/common.h"
#include "lab2/inventory/Asset.h"
#include "lab2/maintenance/MaintenancePlan.h"
#include "lab2/maintenance/RepairEstimate.h"
#include "lab2/maintenance/ServiceLog.h"
#include "lab2/maintenance/ServiceRule.h"

#include <string>

namespace lab2 {

class MaintenanceDesk {
  public:
    explicit MaintenanceDesk(ServiceRule rule);
    MaintenancePlan planFor(Id asset, Day commissioned) const;
    Id start(Asset& asset, Day day, std::string reason);
    void finish(Asset& asset, MaintenancePlan& plan, Id job, Day day, RepairEstimate estimate);
    const ServiceLog& log() const;

  private:
    ServiceRule rule_;
    ServiceLog log_;
};

} // namespace lab2
