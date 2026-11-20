#pragma once

namespace lab2 {

struct InspectionChecklist {
    bool powersOn;
    bool casingIntact;
    bool accessoriesComplete;
    bool passed() const;
    int defects() const;
};

} // namespace lab2
