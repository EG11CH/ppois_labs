# Лабораторная №1: словарь и множество

C++17. Реализованы варианты **1.2 «Англо-русский словарь»** и
**1.4.1 «Неориентированное канторовское множество»**.

## Сборка и запуск

Команды рассчитаны на Ubuntu 24.04. Открывайте терминал в корне
репозитория — там находится главный `CMakeLists.txt`, а не внутри `src`.

```bash
sudo apt-get update
sudo apt-get install -y g++ cmake catch2 doxygen python3-venv cppcheck clang-format ripgrep
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/lab1/dictionary_cli
./build/lab1/set_cli
```

В меню вводите номер отдельной строкой. `0` — выход.
Для сборки только программ добавьте `-DBUILD_TESTING=OFF`: Catch2 тогда не нужен.

## Что где находится

| Папка | Назначение |
|---|---|
| `lab1/include/lab1` | Объявления трёх классов: какие операции доступны |
| `lab1/src` | Реализация этих операций |
| `lab1/cli` | Меню, чтение команд и две функции `main` |
| `lab1/tests` | Проверки классов и сценариев меню на Catch2 |
| `lab1/examples` | Пример файла словаря |
| `lab1/tools` | Три скрипта: покрытие, запуск программ, упаковка релиза |
| `lab1/docs` | Разбор для защиты и запуск готовых программ |
| `lab2`–`lab4` | Места для следующих лабораторных работ |

Начните изучение с [разбора для защиты](lab1/docs/DEFENSE_RU.md).

## Словарь

Собственное двоичное дерево поиска: в узле пара «английское слово — перевод».
Меньшие ключи слева, большие справа; регистр учитывается.

```cpp
lab1::EnglishRussianDictionary dictionary;
dictionary += lab1::EnglishRussianDictionary::Entry{"cat", "кот"};
dictionary["cat"] = "кошка";
dictionary -= "cat";
```

Есть перегрузки `+=` для пар `std::string`, `const char*` и `char*`.
Доступны `size`, `empty`, `contains`, `clear`, `==`, `!=`, `<<`, `>>`.
Повторный или пустой ключ запрещён. Поиск отсутствующего слова вызывает
исключение. Копии независимы. Загрузка заменяет данные только при успехе.

Для пункта меню «Загрузить» укажите `lab1/examples/dictionary.txt`.
Формат файла — количество пар, затем пары строк в кавычках:

```text
2
"cat" "кот"
"ice cream" "мороженое"
```

## Множество

Элемент — строка или другое множество. Повторы удаляются, порядок не важен.
Ввод в меню: `{a, b, {c}, {}}`. Атом с пробелами пишется в кавычках.

```cpp
const lab1::Set first = lab1::Set::fromString("{a, b}");
const lab1::Set second = lab1::Set::fromString("{b, c}");
const lab1::Set united = first + second;
const lab1::Set common = first * second;
const lab1::Set difference = first - second;
const lab1::Set subsets = first.powerSet();
```

Есть также `+=`, `*=`, `-=`, `add`, `remove`, `empty`, `size`, `==`, `!=`,
чтение и запись через потоки. `set[element]` проверяет принадлежность.
`{}` — пустое множество, `{{}}` содержит один элемент — пустое множество.
Булеан по умолчанию ограничен 4096 подмножествами, глубина разбора — 64.

## Покрытие и документация

После тестов в сборке с `ENABLE_COVERAGE=ON`:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r lab1/requirements-dev.txt
python lab1/tools/check_coverage.py build
cmake --build build --target docs
```

Покрытие проверяется для классов, меню и обеих функций `main`.
Отчёты: `build/coverage/index.html` и
`build/lab1/documentation/html/index.html`. В git их не добавляйте.

## GitHub

Работайте в feature-ветке. В `main` изменения попадают через Pull Request
после одобрения преподавателя. Сборочные файлы исключены через `.gitignore`.

CI проверяет тесты, покрытие строго выше 90%, форматирование и Cppcheck;
создаёт документацию и архив статических программ Linux x86-64.
Рекомендация Cppcheck `useStlAlgorithm` отключена намеренно: в учебном коде
оставлены обычные циклы. Остальные выбранные проверки выполняются.

Тег `lab1-v*` публикует проверенный архив в Releases. Допускается релиз
из `feature/lab1-dictionary-set` без слияния в `main`:

```bash
git tag -a lab1-v1.0.1 -m "Lab1: simpler implementation"
git push origin lab1-v1.0.1
```

Создавайте тег **после** отправки коммита в эту feature-ветку.
Тег `lab1-v1.0.0` уже существует; для новой версии нужен новый тег.
Публикация обновлённой версии произойдёт после успешного запуска GitHub Actions.
