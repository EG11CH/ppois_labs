#pragma once

#include <iosfwd>
#include <string>

namespace lab1::cli {

std::string readLine(std::istream& input, std::ostream& output, const std::string& prompt);
bool readCommand(std::istream& input, int& command);
int runDictionaryCli(std::istream& input, std::ostream& output);
int runSetCli(std::istream& input, std::ostream& output);

} // namespace lab1::cli
