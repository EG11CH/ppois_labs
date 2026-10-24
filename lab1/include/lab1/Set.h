#pragma once

#include "lab1/SetElement.h"

#include <cstddef>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <vector>

namespace lab1 {

/** @brief Неупорядоченное канторовское множество без повторяющихся элементов. */
class Set {
  public:
    /** @brief Защита от экспоненциального роста; лимит можно передать явно. */
    static constexpr std::size_t defaultMaxSubsets = 4096;
    static constexpr std::size_t maxParseDepth = 64;

    Set();
    Set(std::initializer_list<SetElement> elements);
    Set(const Set& other);
    Set(Set&& other) noexcept;
    ~Set();
    Set& operator=(const Set& other);
    Set& operator=(Set&& other) noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    /** @brief Возвращает true, только если добавлен новый элемент. */
    bool add(const SetElement& element);
    /** @brief Возвращает true, только если элемент найден и удалён. */
    bool remove(const SetElement& element);
    bool operator[](const SetElement& element) const;

    Set operator+(const Set& other) const;
    Set& operator+=(const Set& other);
    Set operator*(const Set& other) const;
    Set& operator*=(const Set& other);
    Set operator-(const Set& other) const;
    Set& operator-=(const Set& other);
    /** @brief Возвращает множество всех подмножеств, включая пустое и исходное.
     * @throws std::length_error Число подмножеств превышает maxSubsets.
     */
    Set powerSet(std::size_t maxSubsets = defaultMaxSubsets) const;

    bool operator==(const Set& other) const;
    bool operator!=(const Set& other) const;
    void swap(Set& other) noexcept;
    /** @brief Разбирает всю строку; допускает вложенные множества и quoted-атомы.
     * @throws std::invalid_argument Синтаксис неверен или превышена глубина 64.
     */
    static Set fromString(const std::string& text);
    /** @brief Разбирает один атом или одно вложенное множество из всей строки. */
    static SetElement elementFromString(const std::string& text);
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& output, const Set& set);
    /** @brief При неверном вводе устанавливает failbit и сохраняет объект. */
    friend std::istream& operator>>(std::istream& input, Set& set);

  private:
    std::vector<SetElement> elements_;
    static void appendSubsets(std::vector<Set>& subsets, const SetElement& element);
};

} // namespace lab1
