#include "TemporaryFile.h"
#include "TestFramework.h"
#include "lab1/EnglishRussianDictionary.h"

#include <sstream>
#include <stdexcept>
#include <utility>

using Dictionary = lab1::EnglishRussianDictionary;

namespace {
Dictionary makeDictionary(std::initializer_list<Dictionary::Entry> entries) {
    Dictionary result;
    for (const auto& entry : entries) {
        result += entry;
    }
    return result;
}
} // namespace

TEST_CASE("Dictionary starts empty and copies an empty tree", "[dictionary]") {
    Dictionary dictionary;
    const Dictionary copy(dictionary);
    REQUIRE(dictionary.empty());
    REQUIRE(dictionary.size() == 0);
    REQUIRE_FALSE(dictionary.contains("cat"));
    REQUIRE(dictionary == copy);
    dictionary -= "missing";
    REQUIRE(dictionary.empty());
}

TEST_CASE("Dictionary inserts std strings and returns its own reference", "[dictionary]") {
    Dictionary dictionary;
    const Dictionary::Entry entry{"cat", "кот"};
    REQUIRE(&(dictionary += entry) == &dictionary);
    dictionary += Dictionary::Entry{"apple", "яблоко"};
    dictionary += Dictionary::Entry{"tree", "дерево"};
    REQUIRE(dictionary.size() == 3);
    REQUIRE(dictionary.contains("apple"));
    REQUIRE(dictionary.contains("tree"));
    REQUIRE_FALSE(dictionary.contains("dog"));
    REQUIRE(dictionary["cat"] == "кот");
}

TEST_CASE("Dictionary supports both const and mutable C strings", "[dictionary]") {
    Dictionary dictionary;
    dictionary += Dictionary::CStringEntry{"cat", "кот"};
    char word[] = "tree";
    char translation[] = "derevo";
    dictionary += std::make_pair(word, translation);
    word[0] = 'f';
    translation[0] = 'x';
    REQUIRE(dictionary["cat"] == "кот");
    REQUIRE(dictionary["tree"] == "derevo");
    REQUIRE(dictionary.size() == 2);
}

TEST_CASE("Dictionary rejects null strings and empty keys without changing data", "[dictionary]") {
    auto dictionary = makeDictionary({{"cat", "кот"}});
    const Dictionary original(dictionary);
    REQUIRE_THROWS_AS(dictionary += Dictionary::CStringEntry(nullptr, "кот"),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(dictionary += Dictionary::CStringEntry("dog", nullptr),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(dictionary += Dictionary::MutableCStringEntry(nullptr, nullptr),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(dictionary += Dictionary::Entry("", "перевод"), std::invalid_argument);
    REQUIRE(dictionary == original);
}

TEST_CASE("Dictionary rejects a duplicate and changes translations through indexing",
          "[dictionary]") {
    auto dictionary = makeDictionary({{"cat", "кот"}});
    REQUIRE_THROWS_AS(dictionary += Dictionary::Entry("cat", "кошка"), std::invalid_argument);
    REQUIRE(dictionary["cat"] == "кот");
    dictionary["cat"] = "кошка";
    const Dictionary& readOnly = dictionary;
    REQUIRE(readOnly["cat"] == "кошка");
    REQUIRE_THROWS_AS(dictionary["dog"], std::out_of_range);
    REQUIRE_THROWS_AS(readOnly["dog"], std::out_of_range);
    REQUIRE(dictionary.size() == 1);
}

TEST_CASE("Dictionary keys are case sensitive and empty translations round trip", "[dictionary]") {
    auto dictionary = makeDictionary({{"Cat", "кот"}, {"cat", ""}});
    std::stringstream text;
    text << dictionary;
    Dictionary restored;
    text >> restored;
    REQUIRE(restored == dictionary);
    REQUIRE(restored["cat"].empty());
    REQUIRE(restored["Cat"] == "кот");
}

TEST_CASE("Dictionary deletes leaves and the last root", "[dictionary]") {
    auto dictionary = makeDictionary({{"cat", "кот"}, {"apple", "яблоко"}, {"tree", "дерево"}});
    REQUIRE(&(dictionary -= "apple") == &dictionary);
    dictionary -= "tree";
    REQUIRE(dictionary.size() == 1);
    REQUIRE(dictionary["cat"] == "кот");
    dictionary -= "cat";
    dictionary -= "absent";
    REQUIRE(dictionary.empty());
}

TEST_CASE("Dictionary deletes a node with only a left child", "[dictionary]") {
    auto dictionary = makeDictionary({{"dog", "собака"}, {"cat", "кот"}, {"apple", "яблоко"}});
    dictionary -= "cat";
    REQUIRE(dictionary.size() == 2);
    REQUIRE(dictionary["apple"] == "яблоко");
    dictionary -= "dog";
    REQUIRE(dictionary["apple"] == "яблоко");
}

TEST_CASE("Dictionary deletes a node with only a right child", "[dictionary]") {
    auto dictionary = makeDictionary({{"apple", "яблоко"}, {"cat", "кот"}, {"dog", "собака"}});
    dictionary -= "cat";
    REQUIRE(dictionary["dog"] == "собака");
    dictionary -= "apple";
    REQUIRE(dictionary.size() == 1);
    REQUIRE(dictionary["dog"] == "собака");
}

TEST_CASE("Dictionary deletes a root with an immediate successor", "[dictionary]") {
    auto dictionary = makeDictionary({{"dog", "собака"}, {"cat", "кот"}, {"tree", "дерево"}});
    dictionary -= "dog";
    REQUIRE(dictionary.size() == 2);
    REQUIRE_FALSE(dictionary.contains("dog"));
    REQUIRE(dictionary["cat"] == "кот");
    REQUIRE(dictionary["tree"] == "дерево");
}

TEST_CASE("Dictionary deletes a root with a deep successor and preserves successor child",
          "[dictionary]") {
    auto dictionary = makeDictionary({{"m", "M"}, {"a", "A"}, {"z", "Z"}, {"t", "T"}, {"v", "V"}});
    dictionary -= "m";
    REQUIRE(dictionary.size() == 4);
    REQUIRE_FALSE(dictionary.contains("m"));
    REQUIRE(dictionary["a"] == "A");
    REQUIRE(dictionary["t"] == "T");
    REQUIRE(dictionary["v"] == "V");
    REQUIRE(dictionary["z"] == "Z");
}

TEST_CASE("Dictionary equality depends on content instead of tree shape", "[dictionary]") {
    const auto first = makeDictionary({{"b", "B"}, {"a", "A"}, {"c", "C"}});
    auto second = makeDictionary({{"c", "C"}, {"b", "B"}, {"a", "A"}});
    REQUIRE(first == second);
    second["b"] = "changed";
    REQUIRE(first != second);
    second -= "a";
    REQUIRE(first != second);
}

TEST_CASE("Dictionary copy constructor and copy assignment own separate trees", "[dictionary]") {
    const auto original = makeDictionary({{"b", "B"}, {"a", "A"}, {"c", "C"}});
    Dictionary copy(original);
    auto assigned = makeDictionary({{"old", "старый"}});
    REQUIRE(&(assigned = original) == &assigned);
    copy["b"] = "changed";
    assigned -= "a";
    REQUIRE(original["b"] == "B");
    REQUIRE(original.contains("a"));
    assigned = assigned;
    REQUIRE(assigned.size() == 2);
}

TEST_CASE("Dictionary move operations reset the moved source and handle self move",
          "[dictionary]") {
    auto source = makeDictionary({{"cat", "кот"}});
    Dictionary moved(std::move(source));
    REQUIRE(source.empty());
    REQUIRE(moved["cat"] == "кот");
    auto target = makeDictionary({{"old", "старый"}});
    target = std::move(moved);
    REQUIRE(moved.empty());
    REQUIRE(target.size() == 1);
    Dictionary& self = target;
    target = std::move(self);
    REQUIRE(target["cat"] == "кот");
}

TEST_CASE("Dictionary swaps data and clears both branches safely", "[dictionary]") {
    auto first = makeDictionary({{"m", "M"}, {"c", "C"}, {"e", "E"}, {"z", "Z"}});
    auto second = makeDictionary({{"x", "X"}});
    first.swap(second);
    REQUIRE(first["x"] == "X");
    REQUIRE(second.size() == 4);
    second.clear();
    second.clear();
    REQUIRE(second.empty());
}

TEST_CASE("Dictionary round trips quoted strings and writes sorted keys", "[dictionary]") {
    const auto dictionary = makeDictionary({{"z", "слово с пробелами"}, {"a", "кавычки \" и \\"}});
    std::stringstream stream;
    stream << dictionary;
    REQUIRE(stream.str().find("\"a\"") < stream.str().find("\"z\""));
    Dictionary restored;
    stream >> restored;
    REQUIRE_FALSE(stream.fail());
    REQUIRE(restored == dictionary);
}

TEST_CASE("Dictionary invalid streams preserve the existing dictionary", "[dictionary]") {
    const std::vector<std::string> invalid = {"",
                                              "bad",
                                              "-1",
                                              "1000001",
                                              "1",
                                              "1 \"cat\"",
                                              "1 \"\" \"x\"",
                                              "1 cat кот",
                                              "1 \"cat\" кот",
                                              "2 \"cat\" \"кот\" \"cat\" \"другой\"",
                                              "1 \"cat\" \"unterminated"};
    for (const auto& text : invalid) {
        INFO(text);
        auto dictionary = makeDictionary({{"keep", "сохранить"}});
        const Dictionary original(dictionary);
        std::istringstream input(text);
        input >> dictionary;
        REQUIRE(input.fail());
        REQUIRE(dictionary == original);
    }
}

TEST_CASE("Dictionary extracts successive objects from one stream", "[dictionary]") {
    std::istringstream input("1 \"cat\" \"кот\" 0");
    Dictionary first;
    Dictionary second;
    input >> first >> second;
    REQUIRE(first["cat"] == "кот");
    REQUIRE(second.empty());
    REQUIRE_FALSE(input.fail());
}

TEST_CASE("Dictionary loads a file as replacement including trailing whitespace", "[dictionary]") {
    const TemporaryFile file("dictionary_valid", "1\n\"cat\" \"кот\"\n \t\n");
    auto dictionary = makeDictionary({{"old", "старый"}});
    dictionary.loadFromFile(file.path());
    REQUIRE(dictionary.size() == 1);
    REQUIRE(dictionary["cat"] == "кот");
    REQUIRE_FALSE(dictionary.contains("old"));
}

TEST_CASE("Dictionary failed file loads are atomic", "[dictionary]") {
    const TemporaryFile malformed("dictionary_bad", "1\n\"cat\"");
    const TemporaryFile trailing("dictionary_trailing", "0 extra");
    auto dictionary = makeDictionary({{"keep", "сохранить"}});
    const Dictionary original(dictionary);
    REQUIRE_THROWS_AS(dictionary.loadFromFile(malformed.path()), std::runtime_error);
    REQUIRE_THROWS_AS(dictionary.loadFromFile(trailing.path()), std::runtime_error);
    REQUIRE_THROWS_AS(dictionary.loadFromFile("/nonexistent/lab1/dictionary.txt"),
                      std::runtime_error);
    REQUIRE(dictionary == original);
}

TEST_CASE("Dictionary copies compares and clears a skewed tree of 1500 nodes", "[dictionary]") {
    constexpr int wordCount = 1500;
    Dictionary dictionary;
    for (int index = 0; index < wordCount; ++index) {
        const std::string word = std::string(static_cast<std::size_t>(index + 1), 'a');
        dictionary += Dictionary::Entry{word, "перевод"};
    }
    const Dictionary copy(dictionary);
    REQUIRE(copy.size() == static_cast<std::size_t>(wordCount));
    REQUIRE(copy == dictionary);
    dictionary.clear();
    REQUIRE(dictionary.empty());
    REQUIRE_FALSE(copy.empty());
}
