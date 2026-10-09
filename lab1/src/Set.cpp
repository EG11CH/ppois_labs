#include "lab1/Set.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace lab1 {

Set::Set() = default;
Set::Set(const Set& other) = default;
Set::Set(Set&& other) noexcept = default;
Set::~Set() = default;
Set& Set::operator=(const Set& other) = default;
Set& Set::operator=(Set&& other) noexcept = default;

Set::Set(std::initializer_list<SetElement> elements) {
    for (const auto& element : elements) {
        add(element);
    }
}

bool Set::empty() const noexcept {
    return elements_.empty();
}

std::size_t Set::size() const noexcept {
    return elements_.size();
}

bool Set::operator[](const SetElement& element) const {
    return std::find(elements_.begin(), elements_.end(), element) != elements_.end();
}

bool Set::add(const SetElement& element) {
    if ((*this)[element]) {
        return false;
    }
    elements_.push_back(element);
    return true;
}

bool Set::remove(const SetElement& element) {
    const auto found = std::find(elements_.begin(), elements_.end(), element);
    if (found == elements_.end()) {
        return false;
    }
    elements_.erase(found);
    return true;
}

Set Set::operator+(const Set& other) const {
    Set result(*this);
    result += other;
    return result;
}

Set& Set::operator+=(const Set& other) {
    if (this != &other) {
        for (const auto& element : other.elements_) {
            add(element);
        }
    }
    return *this;
}

Set Set::operator*(const Set& other) const {
    Set result;
    for (const auto& element : elements_) {
        if (other[element]) {
            result.add(element);
        }
    }
    return result;
}

Set& Set::operator*=(const Set& other) {
    Set result = *this * other;
    swap(result);
    return *this;
}

Set Set::operator-(const Set& other) const {
    Set result;
    for (const auto& element : elements_) {
        if (!other[element]) {
            result.add(element);
        }
    }
    return result;
}

Set& Set::operator-=(const Set& other) {
    Set result = *this - other;
    swap(result);
    return *this;
}

void Set::appendSubsets(std::vector<Set>& subsets, const SetElement& element) {
    const std::size_t previousSize = subsets.size();
    for (std::size_t index = 0; index < previousSize; ++index) {
        Set extended = subsets[index];
        extended.add(element);
        subsets.push_back(std::move(extended));
    }
}

namespace {
std::size_t checkedSubsetCount(std::size_t elementCount, std::size_t maxSubsets) {
    std::size_t subsetCount = 1;
    for (std::size_t index = 0; index < elementCount; ++index) {
        if (subsetCount > maxSubsets / 2) {
            throw std::length_error("Булеан превышает заданный лимит числа подмножеств");
        }
        subsetCount *= 2;
    }
    if (subsetCount > maxSubsets) {
        throw std::length_error("Лимит числа подмножеств слишком мал");
    }
    return subsetCount;
}
} // namespace

Set Set::powerSet(std::size_t maxSubsets) const {
    const std::size_t subsetCount = checkedSubsetCount(size(), maxSubsets);
    std::vector<Set> subsets(1);
    subsets.reserve(subsetCount);
    for (const auto& element : elements_) {
        appendSubsets(subsets, element);
    }
    Set result;
    result.elements_.reserve(subsetCount);
    for (const auto& subset : subsets) {
        result.elements_.emplace_back(subset);
    }
    return result;
}

bool Set::operator==(const Set& other) const {
    if (size() != other.size()) {
        return false;
    }
    return std::all_of(elements_.begin(), elements_.end(),
                       [&other](const SetElement& element) { return other[element]; });
}

bool Set::operator!=(const Set& other) const {
    return !(*this == other);
}

void Set::swap(Set& other) noexcept {
    elements_.swap(other.elements_);
}

namespace {
Set parseSet(std::istream& input, std::size_t depth);

bool isDelimiter(int character) {
    return character == std::char_traits<char>::eof() || character == ',' || character == '}' ||
           std::isspace(static_cast<unsigned char>(character)) != 0;
}

std::string parseAtom(std::istream& input) {
    std::string atom;
    if (input.peek() == '"') {
        if (!(input >> std::quoted(atom))) {
            throw std::invalid_argument("Незакрытая строка в кавычках");
        }
    } else {
        while (!isDelimiter(input.peek())) {
            const char character = static_cast<char>(input.get());
            if (character == '{' || character == '"' || character == '\\') {
                throw std::invalid_argument("Специальный символ нужно поместить в кавычки");
            }
            atom.push_back(character);
        }
    }
    return atom;
}

SetElement parseElement(std::istream& input, std::size_t depth) {
    input >> std::ws;
    if (input.peek() == '{') {
        const Set nested = parseSet(input, depth);
        return SetElement(nested);
    }
    const std::string atom = parseAtom(input);
    return SetElement(atom);
}

bool consumeSeparator(std::istream& input) {
    input >> std::ws;
    const int separator = input.get();
    if (separator != '}' && separator != ',') {
        throw std::invalid_argument("Ожидалась ',' или '}'");
    }
    return separator == '}';
}

Set parseSet(std::istream& input, std::size_t depth) {
    input >> std::ws;
    if (depth >= Set::maxParseDepth || input.get() != '{') {
        throw std::invalid_argument("Ожидалась '{' или превышена допустимая глубина");
    }
    Set result;
    input >> std::ws;
    if (input.peek() == '}') {
        input.get();
        return result;
    }
    while (true) {
        const SetElement element = parseElement(input, depth + 1);
        result.add(element);
        if (consumeSeparator(input)) {
            return result;
        }
    }
}

void requireEnd(std::istream& input) {
    input >> std::ws;
    if (!input.eof()) {
        throw std::invalid_argument("Лишние символы после объекта");
    }
}
} // namespace

Set Set::fromString(const std::string& text) {
    std::istringstream input(text);
    Set result = parseSet(input, 0);
    requireEnd(input);
    return result;
}

SetElement Set::elementFromString(const std::string& text) {
    std::istringstream input(text);
    SetElement result = parseElement(input, 0);
    requireEnd(input);
    return result;
}

std::istream& operator>>(std::istream& input, Set& set) {
    try {
        Set replacement = parseSet(input, 0);
        set.swap(replacement);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

std::ostream& operator<<(std::ostream& output, const Set& set) {
    std::vector<std::string> representations;
    std::transform(set.elements_.begin(), set.elements_.end(), std::back_inserter(representations),
                   [](const SetElement& element) { return element.toString(); });
    std::sort(representations.begin(), representations.end());
    output << '{';
    bool first = true;
    for (const auto& representation : representations) {
        if (!first) {
            output << ", ";
        }
        output << representation;
        first = false;
    }
    return output << '}';
}

std::string Set::toString() const {
    std::ostringstream output;
    output << *this;
    return output.str();
}

} // namespace lab1
