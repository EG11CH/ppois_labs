#pragma once

#include <string>

namespace lab2 {

struct Category {
    std::string code;
    std::string name;
    bool requiresBriefing;
    void validate() const;
    bool permitsIssue(bool briefingGiven) const;
};

} // namespace lab2
