#include "Console.h"
#include "lab1/EnglishRussianDictionary.h"

#include <fstream>
#include <ostream>
#include <stdexcept>

namespace lab1::cli {
namespace {

enum class DictionaryCommand {
    Add = 1,
    Remove,
    Find,
    Replace,
    Count,
    Show,
    Load,
    Save,
    Copy,
    Compare
};

const std::string dictionaryMenu =
    "\nАНГЛО-РУССКИЙ СЛОВАРЬ\n"
    "1. Добавить слово\n2. Удалить слово\n3. Найти перевод\n4. Заменить перевод\n"
    "5. Количество слов\n6. Показать словарь\n7. Загрузить из файла\n"
    "8. Сохранить в файл\n9. Проверить независимость копий\n"
    "10. Сравнить со словарём из файла\n0. Выход";

void addWord(EnglishRussianDictionary& dictionary, std::istream& input, std::ostream& output) {
    const std::string word = readLine(input, output, "Английское слово: ");
    const std::string translation = readLine(input, output, "Русский перевод: ");
    const EnglishRussianDictionary::Entry entry{word, translation};
    dictionary += entry;
    output << "Слово добавлено\n";
}

void removeWord(EnglishRussianDictionary& dictionary, std::istream& input, std::ostream& output) {
    const std::string word = readLine(input, output, "Удаляемое слово: ");
    const bool existed = dictionary.contains(word);
    dictionary -= word;
    output << (existed ? "Слово удалено\n" : "Такого слова нет\n");
}

void findWord(const EnglishRussianDictionary& dictionary, std::istream& input,
              std::ostream& output) {
    const std::string word = readLine(input, output, "Английское слово: ");
    output << "Перевод: " << dictionary[word] << '\n';
}

void replaceTranslation(EnglishRussianDictionary& dictionary, std::istream& input,
                        std::ostream& output) {
    const std::string word = readLine(input, output, "Английское слово: ");
    const std::string translation = readLine(input, output, "Новый перевод: ");
    dictionary[word] = translation;
    output << "Перевод заменён\n";
}

void loadDictionary(EnglishRussianDictionary& dictionary, std::istream& input,
                    std::ostream& output) {
    const std::string path = readLine(input, output, "Путь к файлу: ");
    dictionary.loadFromFile(path);
    output << "Загружено слов: " << dictionary.size() << '\n';
}

void saveDictionary(const EnglishRussianDictionary& dictionary, std::istream& input,
                    std::ostream& output) {
    const std::string path = readLine(input, output, "Путь к файлу: ");
    std::ofstream file(path);
    file << dictionary;
    file.close();
    if (!file) {
        throw std::runtime_error("Не удалось сохранить файл");
    }
    output << "Словарь сохранён\n";
}

void demonstrateCopy(const EnglishRussianDictionary& dictionary, std::ostream& output) {
    EnglishRussianDictionary copy(dictionary);
    EnglishRussianDictionary assigned;
    assigned = copy;
    output << "Копии равны оригиналу: " << (assigned == dictionary ? "да" : "нет") << '\n';
    copy.clear();
    output << "После очистки копии: копия=" << copy.size() << ", оригинал=" << dictionary.size()
           << '\n';
}

void compareDictionary(const EnglishRussianDictionary& dictionary, std::istream& input,
                       std::ostream& output) {
    const std::string path = readLine(input, output, "Путь ко второму словарю: ");
    EnglishRussianDictionary other;
    other.loadFromFile(path);
    output << "Словари " << (dictionary == other ? "равны\n" : "различаются\n");
}

void executeWordCommand(DictionaryCommand command, EnglishRussianDictionary& dictionary,
                        std::istream& input, std::ostream& output) {
    switch (command) {
    case DictionaryCommand::Add: return addWord(dictionary, input, output);
    case DictionaryCommand::Remove: return removeWord(dictionary, input, output);
    case DictionaryCommand::Find: return findWord(dictionary, input, output);
    case DictionaryCommand::Replace: return replaceTranslation(dictionary, input, output);
    case DictionaryCommand::Count:
        output << "Количество слов: " << dictionary.size() << '\n';
        return;
    default: throw std::invalid_argument("Нет такого пункта меню");
    }
}

void executeCommand(int number, EnglishRussianDictionary& dictionary, std::istream& input,
                    std::ostream& output) {
    const auto command = static_cast<DictionaryCommand>(number);
    switch (command) {
    case DictionaryCommand::Show: output << dictionary; return;
    case DictionaryCommand::Load: return loadDictionary(dictionary, input, output);
    case DictionaryCommand::Save: return saveDictionary(dictionary, input, output);
    case DictionaryCommand::Copy: return demonstrateCopy(dictionary, output);
    case DictionaryCommand::Compare: return compareDictionary(dictionary, input, output);
    default: return executeWordCommand(command, dictionary, input, output);
    }
}
} // namespace

int runDictionaryCli(std::istream& input, std::ostream& output) {
    EnglishRussianDictionary dictionary;
    return runMenu(input, output, dictionaryMenu,
                   [&](int command) { executeCommand(command, dictionary, input, output); });
}

} // namespace lab1::cli
