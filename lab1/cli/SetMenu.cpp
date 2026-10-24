#include "Console.h"
#include "lab1/Set.h"

#include <ostream>
#include <stdexcept>

namespace lab1::cli {
namespace {

enum class SetCommand {
    EnterA = 1,
    EnterB,
    Show,
    Count,
    Add,
    Remove,
    Contains,
    Union,
    Intersection,
    Difference,
    PowerSet,
    Copy,
    Compare
};

const std::string setMenu =
    "\nМНОЖЕСТВО\n1. Ввести A\n2. Ввести B\n3. Показать A и B\n"
    "4. Пустота и мощность A\n5. Добавить элемент в A\n6. Удалить элемент из A\n"
    "7. Принадлежит ли элемент A\n8. Объединить: A += B\n"
    "9. Пересечь: A *= B\n10. Вычесть: A -= B\n11. Построить булеан A\n"
    "12. Проверить независимость копии A\n13. Сравнить A и B\n0. Выход";

void enterSet(Set& target, std::istream& input, std::ostream& output) {
    const std::string text = readLine(input, output, "Множество, например {a, b, {c}, {}}: ");
    target = Set::fromString(text);
    output << "Получено: " << target << '\n';
}

SetElement readElement(std::istream& input, std::ostream& output) {
    const std::string text = readLine(input, output, "Элемент (атом или множество): ");
    return Set::elementFromString(text);
}

void addElement(Set& set, std::istream& input, std::ostream& output) {
    const SetElement element = readElement(input, output);
    const bool added = set.add(element);
    output << (added ? "Элемент добавлен\n" : "Элемент уже существует\n");
}

void removeElement(Set& set, std::istream& input, std::ostream& output) {
    const SetElement element = readElement(input, output);
    const bool removed = set.remove(element);
    output << (removed ? "Элемент удалён\n" : "Элемент не найден\n");
}

void checkElement(const Set& set, std::istream& input, std::ostream& output) {
    const SetElement element = readElement(input, output);
    output << "Принадлежит A: " << (set[element] ? "да\n" : "нет\n");
}

void demonstrateCopy(const Set& set, std::ostream& output) {
    Set copy(set);
    Set assigned;
    assigned = copy;
    copy = Set{};
    output << "Копия по присваиванию равна A: " << (assigned == set ? "да\n" : "нет\n");
    output << "После очистки другой копии: копия=" << copy.size() << ", оригинал=" << set.size()
           << '\n';
}

void unite(Set& left, const Set& right, std::ostream& output) {
    const Set result = left + right;
    output << "A + B = " << result << '\n';
    left += right;
    output << "Теперь A = " << left << '\n';
}

void intersect(Set& left, const Set& right, std::ostream& output) {
    const Set result = left * right;
    output << "A * B = " << result << '\n';
    left *= right;
    output << "Теперь A = " << left << '\n';
}

void subtract(Set& left, const Set& right, std::ostream& output) {
    const Set result = left - right;
    output << "A - B = " << result << '\n';
    left -= right;
    output << "Теперь A = " << left << '\n';
}

void printPowerSet(const Set& set, std::ostream& output) {
    const Set subsets = set.powerSet();
    output << "Булеан: " << subsets << "\nЧисло подмножеств: " << subsets.size() << '\n';
}

void executeElementCommand(SetCommand command, Set& left, Set& right, std::istream& input,
                           std::ostream& output) {
    switch (command) {
    case SetCommand::EnterA: return enterSet(left, input, output);
    case SetCommand::EnterB: return enterSet(right, input, output);
    case SetCommand::Show: output << "A = " << left << "\nB = " << right << '\n'; return;
    case SetCommand::Count:
        output << "A пустое: " << (left.empty() ? "да" : "нет") << "; мощность: " << left.size()
               << '\n';
        return;
    case SetCommand::Add: return addElement(left, input, output);
    case SetCommand::Remove: return removeElement(left, input, output);
    case SetCommand::Contains: return checkElement(left, input, output);
    default: throw std::invalid_argument("Нет такого пункта меню");
    }
}

void executeCommand(int number, Set& left, Set& right, std::istream& input, std::ostream& output) {
    const auto command = static_cast<SetCommand>(number);
    switch (command) {
    case SetCommand::Union: return unite(left, right, output);
    case SetCommand::Intersection: return intersect(left, right, output);
    case SetCommand::Difference: return subtract(left, right, output);
    case SetCommand::PowerSet: return printPowerSet(left, output);
    case SetCommand::Copy: return demonstrateCopy(left, output);
    case SetCommand::Compare:
        output << "A и B " << (left == right ? "равны\n" : "различаются\n");
        return;
    default: return executeElementCommand(command, left, right, input, output);
    }
}
} // namespace

int runSetCli(std::istream& input, std::ostream& output) {
    Set left;
    Set right;
    while (true) {
        output << setMenu << "\nВыбор: ";
        try {
            int command = 0;
            if (!readCommand(input, command) || command == 0) {
                return 0;
            }
            executeCommand(command, left, right, input, output);
        } catch (const std::exception& error) {
            output << "Ошибка: " << error.what() << '\n';
        }
    }
}

} // namespace lab1::cli
