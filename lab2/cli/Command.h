#pragma once

#include <functional>
#include <iosfwd>
#include <string_view>

namespace lab2 {
class RentalDesk;
}

namespace lab2::cli {

struct Command {
    std::string_view name;
    std::string_view arguments;
    std::function<void(std::istream&, std::ostream&, RentalDesk&)> execute;
};

} // namespace lab2::cli
