#pragma once

#include "lab2/common.h"

#include <map>
#include <vector>

namespace lab2 {

enum class AssetState { ready, rented, service, retired };
enum class Condition { good, damaged };

struct Branch {
    Id id;
    std::string name;
    std::string address;
    bool acceptsReturns;
    bool acceptsAt(Id assetBranch) const;
    std::string label() const;
};

struct Category {
    std::string code;
    std::string name;
    bool requiresBriefing;
    void validate() const;
    bool permitsIssue(bool briefingGiven) const;
};

struct AssetModel {
    std::string name;
    std::string manufacturer;
    Category category;
    Money replacementValue;
    void validate() const;
    std::string label() const;
};

class Asset {
  public:
    Asset(Id id, Id branch, std::string serial, AssetModel model);
    Id id() const;
    Id branch() const;
    const std::string& serial() const;
    const AssetModel& model() const;
    AssetState state() const;
    bool canIssue() const;
    void issue();
    void receive(Condition condition);
    void sendToService();
    void releaseFromService();
    void retire();

  private:
    Id id_;
    Id branch_;
    std::string serial_;
    AssetModel model_;
    AssetState state_ = AssetState::ready;
};

struct InspectionChecklist {
    bool powersOn;
    bool casingIntact;
    bool accessoriesComplete;
    bool passed() const;
    int defects() const;
};

struct Inspection {
    Id assetId;
    Day day;
    InspectionChecklist checklist;
    std::string note;
    Condition condition() const;
    void verifyFor(Id asset, Day expectedDay) const;
    void verifyBeforeIssue(Id asset, Day expectedDay) const;
};

class Inventory {
  public:
    void addBranch(Branch branch);
    void addAsset(Asset asset);
    const Branch& branch(Id id) const;
    const Asset& find(Id id) const;
    Asset& edit(Id id);
    std::vector<Id> readyAt(Id branchId) const;
    int count(AssetState state) const;
    std::vector<Id> assetIds() const;

  private:
    std::map<Id, Branch> branches_;
    std::map<Id, Asset> assets_;
};

struct Availability {
    Id assetId;
    Period period;
    bool physicallyReady;
    bool booked;
    bool canReserve() const;
    void requireAvailable() const;
};

} // namespace lab2
