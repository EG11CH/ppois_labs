#pragma once

#include "lab2/common.h"
#include "lab2/maintenance/MaintenanceJob.h"
#include "lab2/maintenance/RepairEstimate.h"

#include <map>
#include <optional>
#include <string>

namespace lab2 {

class ServiceLog {
  public:
    Id openFor(Id asset, Day day, std::string reason);
    const MaintenanceJob& find(Id id) const;
    void close(Id id, Day day, RepairEstimate estimate);
    std::optional<Id> activeFor(Id asset) const;
    Money totalFor(Id asset) const;

  private:
    std::map<Id, MaintenanceJob> jobs_;
    Id nextId_ = 1;
};

} // namespace lab2
