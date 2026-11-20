#pragma once

#include "lab2/common.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Condition.h"

#include <string>

namespace lab2 {

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

} // namespace lab2
