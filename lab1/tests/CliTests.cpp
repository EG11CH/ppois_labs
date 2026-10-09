#include "Console.h"
#include "TemporaryFile.h"
#include "TestFramework.h"

#include <fstream>
#include <sstream>

namespace {
std::string dictionarySession(const std::string& commands) {
    std::istringstream input(commands);
    std::ostringstream output;
    REQUIRE(lab1::cli::runDictionaryCli(input, output) == 0);
    return output.str();
}

std::string setSession(const std::string& commands) {
    std::istringstream input(commands);
    std::ostringstream output;
    REQUIRE(lab1::cli::runSetCli(input, output) == 0);
    return output.str();
}
} // namespace

TEST_CASE("Dictionary menu adds finds replaces counts copies and deletes", "[cli]") {
    const std::string output = dictionarySession(
        "1\ncat\nкот\n3\ncat\n4\ncat\nкошка\n3\ncat\n5\n6\n9\n2\ncat\n2\ndog\n0\n");
    REQUIRE(output.find("Перевод: кот") != std::string::npos);
    REQUIRE(output.find("Перевод: кошка") != std::string::npos);
    REQUIRE(output.find("Количество слов: 1") != std::string::npos);
    REQUIRE(output.find("копия=0, оригинал=1") != std::string::npos);
    REQUIRE(output.find("Слово удалено") != std::string::npos);
    REQUIRE(output.find("Такого слова нет") != std::string::npos);
}

TEST_CASE("Dictionary menu loads saves and compares files", "[cli]") {
    const TemporaryFile source("cli_source", "1\n\"cat\" \"кот\"\n");
    const TemporaryFile target("cli_target", "");
    const TemporaryFile different("cli_different", "0\n");
    const std::string commands = "7\n" + source.path() + "\n8\n" + target.path() + "\n10\n" +
                                 source.path() + "\n10\n" + different.path() + "\n0\n";
    const std::string output = dictionarySession(commands);
    REQUIRE(output.find("Загружено слов: 1") != std::string::npos);
    REQUIRE(output.find("Словарь сохранён") != std::string::npos);
    REQUIRE(output.find("Словари равны") != std::string::npos);
    REQUIRE(output.find("Словари различаются") != std::string::npos);
    std::ifstream saved(target.path());
    std::string firstLine;
    std::getline(saved, firstLine);
    REQUIRE(firstLine == "1");
}

TEST_CASE("Dictionary menu reports user errors and continues", "[cli]") {
    const std::string output =
        dictionarySession("bad\n2x\n99\n3\nmissing\n4\nmissing\nx\n1\ncat\nкот\n1\ncat\nдубль\n"
                          "7\n/nonexistent/lab1/input\n8\n/nonexistent/lab1/output\n5\n0\n");
    REQUIRE(output.find("Введите номер") != std::string::npos);
    REQUIRE(output.find("лишние символы") != std::string::npos);
    REQUIRE(output.find("Нет такого пункта") != std::string::npos);
    REQUIRE(output.find("Слово не найдено") != std::string::npos);
    REQUIRE(output.find("уже есть в словаре") != std::string::npos);
    REQUIRE(output.find("Не удалось сохранить") != std::string::npos);
    REQUIRE(output.find("Количество слов: 1") != std::string::npos);
}

TEST_CASE("Both menus exit on EOF and report an interrupted operation", "[cli]") {
    REQUIRE(dictionarySession("").find("СЛОВАРЬ") != std::string::npos);
    REQUIRE(setSession("").find("МНОЖЕСТВО") != std::string::npos);
    REQUIRE(dictionarySession("1\ncat\n").find("Ввод завершён") != std::string::npos);
    REQUIRE(setSession("1\n").find("Ввод завершён") != std::string::npos);
}

TEST_CASE("Set menu edits nested elements and handles duplicates", "[cli]") {
    const std::string output =
        setSession("4\n1\n{a, {b}, {}}\n2\n{{}, {b}, a}\n3\n4\n13\n12\n5\na\n5\nc\n"
                   "7\nc\n7\nz\n6\nc\n6\nz\n13\n0\n");
    REQUIRE(output.find("A пустое: да; мощность: 0") != std::string::npos);
    REQUIRE(output.find("A пустое: нет; мощность: 3") != std::string::npos);
    REQUIRE(output.find("A и B равны") != std::string::npos);
    REQUIRE(output.find("Элемент уже существует") != std::string::npos);
    REQUIRE(output.find("Элемент добавлен") != std::string::npos);
    REQUIRE(output.find("Принадлежит A: да") != std::string::npos);
    REQUIRE(output.find("Принадлежит A: нет") != std::string::npos);
    REQUIRE(output.find("Элемент удалён") != std::string::npos);
    REQUIRE(output.find("Элемент не найден") != std::string::npos);
}

TEST_CASE("Set menu demonstrates all algebra operations and the boolean", "[cli]") {
    const std::string output = setSession("1\n{a, b}\n2\n{b, c}\n13\n8\n9\n10\n1\n{a, b}\n11\n0\n");
    REQUIRE(output.find("A и B различаются") != std::string::npos);
    REQUIRE(output.find("A + B = {a, b, c}") != std::string::npos);
    REQUIRE(output.find("A * B = {b, c}") != std::string::npos);
    REQUIRE(output.find("A - B = {}") != std::string::npos);
    REQUIRE(output.find("Число подмножеств: 4") != std::string::npos);
}

TEST_CASE("Set menu rejects invalid objects without losing its previous set", "[cli]") {
    const std::string output = setSession("1\n{a}\n1\n{a,}\n5\n\n99\n3\n0\n");
    REQUIRE(output.find("Ошибка:") != std::string::npos);
    REQUIRE(output.find("Нет такого пункта") != std::string::npos);
    REQUIRE(output.find("A = {a}") != std::string::npos);
}
