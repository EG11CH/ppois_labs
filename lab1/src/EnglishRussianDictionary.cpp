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
    : root_(std::move(other.root_)), wordCount_(std::exchange(other.wordCount_, 0)) {
}

EnglishRussianDictionary::~EnglishRussianDictionary() {
    clear();
}

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
        wordCount_ = std::exchange(other.wordCount_, 0);
    }
    return *this;
}

std::unique_ptr<EnglishRussianDictionary::Node>*
EnglishRussianDictionary::findLink(const std::string& englishWord) noexcept {
    auto* link = &root_;
    while (*link && (*link)->entry.first != englishWord) {
        const bool goLeft = englishWord < (*link)->entry.first;
        link = goLeft ? &(*link)->left : &(*link)->right;
    }
    return link;
}

const EnglishRussianDictionary::Node*
EnglishRussianDictionary::findNode(const std::string& englishWord) const noexcept {
    const Node* current = root_.get();
    while (current && current->entry.first != englishWord) {
        const bool goLeft = englishWord < current->entry.first;
        current = goLeft ? current->left.get() : current->right.get();
    }
    return current;
}

EnglishRussianDictionary& EnglishRussianDictionary::operator+=(const Entry& entry) {
    if (entry.first.empty()) {
        throw std::invalid_argument("Английское слово не может быть пустым");
    }
    auto* link = findLink(entry.first);
    if (*link) {
        throw std::invalid_argument("Это слово уже есть в словаре");
    }
    *link = std::make_unique<Node>(entry);
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

std::unique_ptr<EnglishRussianDictionary::Node>*
EnglishRussianDictionary::prepareRemoval(std::unique_ptr<Node>* link) noexcept {
    if (!(*link)->left || !(*link)->right) {
        return link;
    }
    auto* successor = &(*link)->right;
    while ((*successor)->left) {
        successor = &(*successor)->left;
    }
    (*link)->entry.swap((*successor)->entry);
    return successor;
}

EnglishRussianDictionary& EnglishRussianDictionary::operator-=(const std::string& englishWord) {
    auto* link = findLink(englishWord);
    if (!*link) {
        return *this;
    }
    link = prepareRemoval(link);
    auto removed = std::move(*link);
    *link = removed->left ? std::move(removed->left) : std::move(removed->right);
    --wordCount_;
    return *this;
}

std::string& EnglishRussianDictionary::operator[](const std::string& englishWord) {
    auto* link = findLink(englishWord);
    if (!*link) {
        throw std::out_of_range("Слово не найдено");
    }
    return (*link)->entry.second;
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
    while (root_) {
        if (root_->left) {
            auto leftChild = std::move(root_->left);
            root_->left = std::move(leftChild->right);
            leftChild->right = std::move(root_);
            root_ = std::move(leftChild);
        } else {
            auto removed = std::move(root_);
            root_ = std::move(removed->right);
        }
    }
    wordCount_ = 0;
}

void EnglishRussianDictionary::swap(EnglishRussianDictionary& other) noexcept {
    root_.swap(other.root_);
    std::swap(wordCount_, other.wordCount_);
}

std::unique_ptr<EnglishRussianDictionary::Node>
EnglishRussianDictionary::cloneTree(const std::unique_ptr<Node>& source) {
    if (!source) {
        return nullptr;
    }
    auto copy = std::make_unique<Node>(source->entry);
    std::vector<std::pair<const Node*, Node*>> pending{{source.get(), copy.get()}};
    while (!pending.empty()) {
        const auto [original, cloned] = pending.back();
        pending.pop_back();
        cloneChildren(original, cloned, pending);
    }
    return copy;
}

void EnglishRussianDictionary::cloneChildren(const Node* original, Node* cloned,
                                             std::vector<std::pair<const Node*, Node*>>& pending) {
    for (const bool leftSide : {true, false}) {
        const auto& child = leftSide ? original->left : original->right;
        if (child) {
            auto& target = leftSide ? cloned->left : cloned->right;
            target = std::make_unique<Node>(child->entry);
            pending.emplace_back(child.get(), target.get());
        }
    }
}

std::vector<EnglishRussianDictionary::Entry> EnglishRussianDictionary::orderedEntries() const {
    std::vector<Entry> result;
    std::vector<const Node*> ancestors;
    const Node* current = root_.get();
    while (current || !ancestors.empty()) {
        while (current) {
            ancestors.push_back(current);
            current = current->left.get();
        }
        current = ancestors.back();
        ancestors.pop_back();
        result.push_back(current->entry);
        current = current->right.get();
    }
    return result;
}

bool EnglishRussianDictionary::operator==(const EnglishRussianDictionary& other) const {
    return wordCount_ == other.wordCount_ && orderedEntries() == other.orderedEntries();
}

bool EnglishRussianDictionary::operator!=(const EnglishRussianDictionary& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const EnglishRussianDictionary& dictionary) {
    output << dictionary.wordCount_ << '\n';
    for (const auto& [word, translation] : dictionary.orderedEntries()) {
        output << std::quoted(word) << ' ' << std::quoted(translation) << '\n';
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
