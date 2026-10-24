#include "lab1/EnglishRussianDictionary.h"

#include <fstream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace lab1 {

struct EnglishRussianDictionary::Node {
    explicit Node(const Entry& newEntry) : entry(newEntry) {
    }
    Entry entry;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

EnglishRussianDictionary::EnglishRussianDictionary() = default;

EnglishRussianDictionary::EnglishRussianDictionary(const EnglishRussianDictionary& other)
    : root_(cloneTree(other.root_)), wordCount_(other.wordCount_) {
}

EnglishRussianDictionary::EnglishRussianDictionary(EnglishRussianDictionary&& other) noexcept
    : root_(std::move(other.root_)), wordCount_(other.wordCount_) {
    other.wordCount_ = 0;
}

EnglishRussianDictionary::~EnglishRussianDictionary() = default;

EnglishRussianDictionary&
EnglishRussianDictionary::operator=(const EnglishRussianDictionary& other) {
    if (this != &other) {
        EnglishRussianDictionary replacement(other);
        swap(replacement);
    }
    return *this;
}

EnglishRussianDictionary&
EnglishRussianDictionary::operator=(EnglishRussianDictionary&& other) noexcept {
    if (this != &other) {
        clear();
        root_ = std::move(other.root_);
        wordCount_ = other.wordCount_;
        other.wordCount_ = 0;
    }
    return *this;
}

// Поиск идёт влево для меньшего ключа и вправо для большего.
EnglishRussianDictionary::Node*
EnglishRussianDictionary::findNode(const std::string& englishWord) const noexcept {
    Node* current = root_.get();
    while (current != nullptr) {
        if (englishWord == current->entry.first) {
            return current;
        }
        if (englishWord < current->entry.first) {
            current = current->left.get();
        } else {
            current = current->right.get();
        }
    }
    return nullptr;
}

// Ссылка на unique_ptr позволяет изменить нужную ветвь дерева.
void EnglishRussianDictionary::insertNode(std::unique_ptr<Node>& node, const Entry& entry) {
    if (node == nullptr) {
        node = std::make_unique<Node>(entry);
    } else if (entry.first < node->entry.first) {
        insertNode(node->left, entry);
    } else if (entry.first > node->entry.first) {
        insertNode(node->right, entry);
    } else {
        throw std::invalid_argument("Это слово уже есть в словаре");
    }
}

EnglishRussianDictionary& EnglishRussianDictionary::operator+=(const Entry& entry) {
    if (entry.first.empty()) {
        throw std::invalid_argument("Английское слово не может быть пустым");
    }
    insertNode(root_, entry);
    ++wordCount_;
    return *this;
}

EnglishRussianDictionary& EnglishRussianDictionary::operator+=(const CStringEntry& entry) {
    if (!entry.first || !entry.second) {
        throw std::invalid_argument("C-строка не может быть nullptr");
    }
    const Entry strings{entry.first, entry.second};
    return *this += strings;
}

EnglishRussianDictionary& EnglishRussianDictionary::operator+=(const MutableCStringEntry& entry) {
    const CStringEntry readOnlyStrings{entry.first, entry.second};
    return *this += readOnlyStrings;
}

// Для узла с двумя потомками берём следующее по порядку слово справа.
void EnglishRussianDictionary::removeRoot(std::unique_ptr<Node>& node) {
    if (node->left != nullptr && node->right != nullptr) {
        Node* successor = node->right.get();
        while (successor->left != nullptr) {
            successor = successor->left.get();
        }
        const std::string oldWord = node->entry.first;
        node->entry.swap(successor->entry);
        removeNode(node->right, oldWord);
    } else {
        std::unique_ptr<Node> removed = std::move(node);
        if (removed->left != nullptr) {
            node = std::move(removed->left);
        } else {
            node = std::move(removed->right);
        }
    }
}

bool EnglishRussianDictionary::removeNode(std::unique_ptr<Node>& node,
                                          const std::string& englishWord) {
    if (node == nullptr) {
        return false;
    }
    if (englishWord < node->entry.first) {
        return removeNode(node->left, englishWord);
    }
    if (englishWord > node->entry.first) {
        return removeNode(node->right, englishWord);
    }
    removeRoot(node);
    return true;
}

EnglishRussianDictionary& EnglishRussianDictionary::operator-=(const std::string& englishWord) {
    if (removeNode(root_, englishWord)) {
        --wordCount_;
    }
    return *this;
}

std::string& EnglishRussianDictionary::operator[](const std::string& englishWord) {
    Node* found = findNode(englishWord);
    if (found == nullptr) {
        throw std::out_of_range("Слово не найдено");
    }
    return found->entry.second;
}

const std::string& EnglishRussianDictionary::operator[](const std::string& englishWord) const {
    const Node* found = findNode(englishWord);
    if (!found) {
        throw std::out_of_range("Слово не найдено");
    }
    return found->entry.second;
}

std::size_t EnglishRussianDictionary::size() const noexcept {
    return wordCount_;
}

bool EnglishRussianDictionary::empty() const noexcept {
    return wordCount_ == 0;
}

bool EnglishRussianDictionary::contains(const std::string& englishWord) const noexcept {
    return findNode(englishWord) != nullptr;
}

void EnglishRussianDictionary::clear() noexcept {
    root_.reset();
    wordCount_ = 0;
}

void EnglishRussianDictionary::swap(EnglishRussianDictionary& other) noexcept {
    root_.swap(other.root_);
    std::swap(wordCount_, other.wordCount_);
}

// Каждый узел копируется вместе с левым и правым поддеревом.
std::unique_ptr<EnglishRussianDictionary::Node>
EnglishRussianDictionary::cloneTree(const std::unique_ptr<Node>& source) {
    if (source == nullptr) {
        return nullptr;
    }
    std::unique_ptr<Node> copy = std::make_unique<Node>(source->entry);
    copy->left = cloneTree(source->left);
    copy->right = cloneTree(source->right);
    return copy;
}

// Симметричный обход: левое поддерево, текущий узел, правое поддерево.
void EnglishRussianDictionary::collectEntries(const Node* node, std::vector<Entry>& entries) {
    if (node == nullptr) {
        return;
    }
    collectEntries(node->left.get(), entries);
    entries.push_back(node->entry);
    collectEntries(node->right.get(), entries);
}

std::vector<EnglishRussianDictionary::Entry> EnglishRussianDictionary::orderedEntries() const {
    std::vector<Entry> entries;
    collectEntries(root_.get(), entries);
    return entries;
}

bool EnglishRussianDictionary::operator==(const EnglishRussianDictionary& other) const {
    return wordCount_ == other.wordCount_ && orderedEntries() == other.orderedEntries();
}

bool EnglishRussianDictionary::operator!=(const EnglishRussianDictionary& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const EnglishRussianDictionary& dictionary) {
    output << dictionary.wordCount_ << '\n';
    const std::vector<EnglishRussianDictionary::Entry> entries = dictionary.orderedEntries();
    for (const auto& entry : entries) {
        output << std::quoted(entry.first) << ' ' << std::quoted(entry.second) << '\n';
    }
    return output;
}

namespace {
bool readQuotedString(std::istream& input, std::string& value) {
    input >> std::ws;
    if (input.peek() != '"') {
        return false;
    }
    return static_cast<bool>(input >> std::quoted(value));
}

bool readEntries(std::istream& input, EnglishRussianDictionary& result) {
    constexpr long long maxFileEntries = 1'000'000;
    long long expectedCount = 0;
    if (!(input >> expectedCount) || expectedCount < 0 || expectedCount > maxFileEntries) {
        return false;
    }
    for (long long index = 0; index < expectedCount; ++index) {
        EnglishRussianDictionary::Entry entry;
        if (!readQuotedString(input, entry.first) || !readQuotedString(input, entry.second)) {
            return false;
        }
        result += entry;
    }
    return true;
}
} // namespace

// Ошибка чтения не должна уничтожать прежнее содержимое словаря.
std::istream& operator>>(std::istream& input, EnglishRussianDictionary& dictionary) {
    EnglishRussianDictionary replacement;
    try {
        if (readEntries(input, replacement)) {
            dictionary.swap(replacement);
            return input;
        }
    } catch (const std::invalid_argument&) {
        // Повтор ключа и пустой ключ означают неверный формат входных данных.
    }
    input.setstate(std::ios::failbit);
    return input;
}

void EnglishRussianDictionary::loadFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    EnglishRussianDictionary replacement;
    if (!input.is_open() || !(input >> replacement)) {
        throw std::runtime_error("Не удалось загрузить словарь: " + filePath);
    }
    input >> std::ws;
    if (!input.eof() || input.bad()) {
        throw std::runtime_error("Лишние данные в файле словаря");
    }
    swap(replacement);
}

} // namespace lab1
