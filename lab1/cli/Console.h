#pragma once

#include <functional>
#include <iosfwd>
#include <string>

namespace lab1::cli {

std::string readLine(std::istream& input, std::ostream& output, const std::string& prompt);
int runMenu(std::istream& input, std::ostream& output, const std::string& menu,
            const std::function<void(int)>& execute);
int runDictionaryCli(std::istream& input, std::ostream& output);
int runSetCli(std::istream& input, std::ostream& output);

} // namespace lab1::cli
