#pragma once

#include "lab2/common.h"

namespace lab2 {

class Period {
  public:
    Period(Day start, Day end);
    bool contains(Day day) const;
    bool overlaps(const Period& other) const;
    bool covers(const Period& other) const;
    int days() const;

    Day start() const;
    Day end() const;

  private:
    Day start_;
    Day end_;
};

} // namespace lab2
