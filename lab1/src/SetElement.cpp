#include "lab1/SetElement.h"
#include "lab1/Set.h"

#include <cctype>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace lab1 {

SetElement::SetElement(std::string atom) : atom_(std::move(atom)) {
    if (atom_.empty()) {
        throw std::invalid_argument("Атом не может быть пустой строкой");
    }
}

SetElement::SetElement(const Set& nestedSet) : nestedSet_(std::make_unique<Set>(nestedSet)) {
}

SetElement::SetElement(const SetElement& other) : atom_(other.atom_) {
    // Создаём собственное вложенное множество, а не копируем его адрес.
    if (other.isSet()) {
        nestedSet_ = std::make_unique<Set>(*other.nestedSet_);
    }
}

SetElement::SetElement(SetElement&& other) noexcept = default;
SetElement::~SetElement() = default;

SetElement& SetElement::operator=(const SetElement& other) {
    if (this != &other) {
        SetElement replacement(other);
        swap(replacement);
    }
    return *this;
}

SetElement& SetElement::operator=(SetElement&& other) noexcept = default;

bool SetElement::isSet() const noexcept {
    return nestedSet_ != nullptr;
}

const std::string& SetElement::asAtom() const {
    if (isSet()) {
        throw std::logic_error("Этот элемент — множество, а не атом");
    }
    return atom_;
}

const Set& SetElement::asSet() const {
    if (!isSet()) {
        throw std::logic_error("Этот элемент — атом, а не множество");
    }
    return *nestedSet_;
}

void SetElement::swap(SetElement& other) noexcept {
    atom_.swap(other.atom_);
    nestedSet_.swap(other.nestedSet_);
}

bool SetElement::operator==(const SetElement& other) const {
    if (isSet() != other.isSet()) {
        return false;
    }
    if (isSet()) {
        return *nestedSet_ == *other.nestedSet_;
    }
    return atom_ == other.atom_;
}

bool SetElement::operator!=(const SetElement& other) const {
    return !(*this == other);
}

namespace {
bool needsQuotes(const std::string& atom) {
    const std::string specialCharacters = "{},\"\\";
    for (char character : atom) {
        const bool space = std::isspace(static_cast<unsigned char>(character)) != 0;
        if (space || specialCharacters.find(character) != std::string::npos) {
            return true;
        }
    }
    return false;
}
} // namespace

std::ostream& operator<<(std::ostream& output, const SetElement& element) {
    if (element.isSet()) {
        return output << element.asSet();
    }
    if (needsQuotes(element.atom_)) {
        return output << std::quoted(element.atom_);
    }
    return output << element.atom_;
}

std::string SetElement::toString() const {
    std::ostringstream output;
    output << *this;
    return output.str();
}

SetElement SetElement::fromString(const std::string& text) {
    return Set::elementFromString(text);
}

} // namespace lab1
