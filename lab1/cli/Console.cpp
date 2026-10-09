#include "Console.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace lab1::cli {

std::string readLine(std::istream& input, std::ostream& output, const std::string& prompt) {
    output << prompt;
    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Ввод завершён");
    }
    return line;
}

namespace {
int parseCommand(const std::string& line) {
    std::istringstream parser(line);
    int command = 0;
    if (!(parser >> command)) {
        throw std::invalid_argument("Введите номер пункта меню");
    }
    parser >> std::ws;
    if (!parser.eof()) {
        throw std::invalid_argument("После номера команды есть лишние символы");
    }
    return command;
}
bool executeLine(const std::string& line, std::ostream& output,
                 const std::function<void(int)>& execute) {
    constexpr int exitCommand = 0;
    try {
        const int command = parseCommand(line);
        if (command == exitCommand) {
            return false;
        }
        execute(command);
    } catch (const std::exception& error) {
        output << "Ошибка: " << error.what() << '\n';
    }
    return true;
}
} // namespace

int runMenu(std::istream& input, std::ostream& output, const std::string& menu,
            const std::function<void(int)>& execute) {
    while (true) {
        output << menu << "\nВыбор: ";
        std::string line;
        if (!std::getline(input, line) || !executeLine(line, output, execute)) {
            return 0;
        }
    }
}

} // namespace lab1::cli
