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

// false означает конец ввода; неверная команда вызывает исключение.
bool readCommand(std::istream& input, int& command) {
    std::string line;
    if (!std::getline(input, line)) {
        return false;
    }
    std::istringstream parser(line);
    if (!(parser >> command)) {
        throw std::invalid_argument("Введите номер пункта меню");
    }
    parser >> std::ws;
    if (!parser.eof()) {
        throw std::invalid_argument("После номера команды есть лишние символы");
    }
    return true;
}

} // namespace lab1::cli
