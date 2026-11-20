#pragma once

#include "lab2/inventory.h"

#include <optional>

namespace lab2 {

struct ServiceRule {
    int maximumCycles;
    int maximumDays;
    bool inspectAfterDamage;
    bool due(int cycles, Day previous, Day today, Condition condition) const;
    void validate() const;
};

struct MaintenancePlan {
    Id assetId;
    ServiceRule rule;
    Day lastService;
    int rentalCycles;
    bool dueOn(Day day) const;
    bool recordReturn(Day day, Condition condition);
    void reset(Day day);
};

struct Part {
    std::string sku;
    std::string name;
    std::string supplier;
    Money unitPrice;
    void validate() const;
    Money priceFor(int quantity) const;
};

struct PartUsage {
    Part part;
    int quantity;
    std::string reason;
    Money cost() const;
    std::string summary() const;
};

struct RepairEstimate {
    Money labor;
    std::vector<PartUsage> parts;
    Money limit;
    Money total() const;
    void requireWithinBudget() const;
};

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
