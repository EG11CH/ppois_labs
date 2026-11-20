#pragma once

#include "lab2/common.h"
#include "lab2/maintenance/RepairEstimate.h"

#include <optional>
#include <string>

namespace lab2 {

class MaintenanceJob {
  public:
    MaintenanceJob(Id id, Id assetId, Day opened, std::string reason);
    Id id() const;
    Id assetId() const;
    bool open() const;
    void finish(Day day, RepairEstimate estimate);
    Money expense() const;
    std::string summary() const;

  private:
    Id id_;
    Id assetId_;
    Day opened_;
    std::string reason_;
    std::optional<Day> closed_;
    std::optional<RepairEstimate> estimate_;
};

} // namespace lab2
