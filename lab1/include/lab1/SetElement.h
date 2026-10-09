#pragma once

#include <iosfwd>
#include <memory>
#include <string>

namespace lab1 {
class Set;

/** @brief Элемент множества: строковый атом или независимое вложенное множество.
 * Вложенный объект доступен только для чтения, поэтому его нельзя изменить,
 * нарушив уникальность элементов содержащего множества.
 */
class SetElement {
  public:
    explicit SetElement(std::string atom);
    explicit SetElement(const Set& nestedSet);
    SetElement(const SetElement& other);
    SetElement(SetElement&& other) noexcept;
    ~SetElement();
    SetElement& operator=(const SetElement& other);
    SetElement& operator=(SetElement&& other) noexcept;

    bool isSet() const noexcept;
    /** @throws std::logic_error Если элемент является множеством. */
    const std::string& asAtom() const;
    /** @throws std::logic_error Если элемент является атомом. */
    const Set& asSet() const;
    /** @brief Создаёт элемент из всей строки (атом или вложенное множество). */
    static SetElement fromString(const std::string& text);
    std::string toString() const;
    void swap(SetElement& other) noexcept;
    bool operator==(const SetElement& other) const;
    bool operator!=(const SetElement& other) const;
    friend std::ostream& operator<<(std::ostream& output, const SetElement& element);

  private:
    std::string atom_;
    std::unique_ptr<Set> nestedSet_;
};

} // namespace lab1
