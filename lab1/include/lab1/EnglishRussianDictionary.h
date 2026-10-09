#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace lab1 {

/** @brief Англо-русский словарь на собственном двоичном дереве поиска.
 * Ключи чувствительны к регистру. Повторное добавление — ошибка.
 * Поиск отсутствующего слова бросает std::out_of_range.
 */
class EnglishRussianDictionary {
  public:
    using Entry = std::pair<std::string, std::string>;
    using CStringEntry = std::pair<const char*, const char*>;
    using MutableCStringEntry = std::pair<char*, char*>;

    EnglishRussianDictionary();
    /** @brief Создаёт независимую глубокую копию дерева. */
    EnglishRussianDictionary(const EnglishRussianDictionary& other);
    EnglishRussianDictionary(EnglishRussianDictionary&& other) noexcept;
    ~EnglishRussianDictionary();
    EnglishRussianDictionary& operator=(const EnglishRussianDictionary& other);
    EnglishRussianDictionary& operator=(EnglishRussianDictionary&& other) noexcept;

    /** @brief Добавляет пару std::string; пустой ключ или повтор запрещены. */
    EnglishRussianDictionary& operator+=(const Entry& entry);
    /** @brief Добавляет пару C-строк; nullptr запрещён. */
    EnglishRussianDictionary& operator+=(const CStringEntry& entry);
    /** @brief Перегрузка для пары изменяемых char*. */
    EnglishRussianDictionary& operator+=(const MutableCStringEntry& entry);
    /** @brief Удаляет слово. Если его нет, словарь не меняется. */
    EnglishRussianDictionary& operator-=(const std::string& englishWord);
    /** @brief Возвращает перевод существующего слова для чтения/изменения. */
    std::string& operator[](const std::string& englishWord);
    const std::string& operator[](const std::string& englishWord) const;

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    bool contains(const std::string& englishWord) const noexcept;
    void clear() noexcept;
    void swap(EnglishRussianDictionary& other) noexcept;
    /** @brief Заменяет словарь данными UTF-8-файла; при ошибке не меняет его.
     * @throws std::runtime_error Файл не открыт или его формат неверен.
     */
    void loadFromFile(const std::string& filePath);

    bool operator==(const EnglishRussianDictionary& other) const;
    bool operator!=(const EnglishRussianDictionary& other) const;
    /** @brief Формат: количество, затем пары строк в двойных кавычках. */
    friend std::ostream& operator<<(std::ostream& output,
                                    const EnglishRussianDictionary& dictionary);
    /** @brief Читает один объект; при ошибке устанавливает failbit. */
    friend std::istream& operator>>(std::istream& input, EnglishRussianDictionary& dictionary);

  private:
    struct Node;
    std::unique_ptr<Node> root_;
    std::size_t wordCount_ = 0;

    std::unique_ptr<Node>* findLink(const std::string& englishWord) noexcept;
    const Node* findNode(const std::string& englishWord) const noexcept;
    static std::unique_ptr<Node>* prepareRemoval(std::unique_ptr<Node>* link) noexcept;
    static std::unique_ptr<Node> cloneTree(const std::unique_ptr<Node>& source);
    static void cloneChildren(const Node* original, Node* cloned,
                              std::vector<std::pair<const Node*, Node*>>& pending);
    std::vector<Entry> orderedEntries() const;
};

} // namespace lab1
