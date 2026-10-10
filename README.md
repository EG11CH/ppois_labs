# Лабораторная работа №1

Реализации на C++17 для вариантов 1.2 «Англо-русский словарь» и
1.4.1 «Множество». Проект содержит две консольные программы, библиотеку
классов, модульные тесты и настройки CI.

## Сборка на Linux

Проверенная среда: Ubuntu 24.04, GCC 13.3, Catch2 3.4.
Требуются GCC с поддержкой C++17, CMake >= 3.20 и Catch2 с CMake-целью
`Catch2::Catch2WithMain`. Для тестов запуска программ нужен Python 3.

Установка инструментов для Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y g++ cmake catch2 doxygen python3 python3-venv cppcheck clang-format ripgrep
```

Команды из корня репозитория:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/lab1/dictionary_cli
./build/lab1/set_cli
```

Для сборки только программ можно добавить `-DBUILD_TESTING=OFF`; тогда
Catch2 не требуется. Данные меню и пример словаря используют UTF-8.

## Структура

| Каталог | Назначение |
|---|---|
| `lab1/include/lab1` | Публичные объявления классов |
| `lab1/src` | Реализации словаря, множества и элемента множества |
| `lab1/cli` | Консольные меню и обработка ввода |
| `lab1/app` | Точки входа программ |
| `lab1/tests` | Тесты Catch2 и сценарии меню |
| `lab1/examples` | Пример словаря |
| `lab1/tools` | Проверка покрытия, запуск программ и упаковка релиза |
| `lab1/docs` | Документация и результаты локальной проверки |
| `lab2`, `lab3`, `lab4` | Каталоги для следующих лабораторных |

Описание операций, форматов и ограничений: [lab1/README.md](lab1/README.md).
Соответствие заданию: [REQUIREMENTS.md](lab1/docs/REQUIREMENTS.md).
Результаты локальных проверок: [VERIFICATION_RU.md](lab1/docs/VERIFICATION_RU.md).

## Тесты, покрытие и документация

В тестах проверяются обычные операции, ошибки ввода, независимость копий,
удаление узлов дерева, вложенные множества и работа консольных меню.
Дополнительный тест запускает оба исполняемых файла.

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r lab1/requirements-dev.txt
python lab1/tools/check_coverage.py build
cmake --build build --target docs
```

Перед измерением покрытия требуется выполнить тесты в сборке с
`ENABLE_COVERAGE=ON`. Скрипт требует покрытие строк строго больше 90%.
Он включает классы, меню и точки входа; тесты и Catch2 исключены из измерения.
HTML-отчёт: `build/coverage/index.html`.
HTML Doxygen: `build/lab1/documentation/html/index.html`.

После изменения исходников для нового измерения используйте новую папку
сборки. Результаты сборки и отчёты исключены из git.

## GitHub Actions

Workflow `.github/workflows/lab1.yml` выполняет сборку, тесты, проверку
покрытия, Cppcheck, проверку форматирования и генерацию Doxygen. Затем
собирает статические программы Linux x86-64 и публикует архив как artifact.

Работа добавляется в существующий общий репозиторий через отдельную ветку
`feature/lab1-dictionary-set`. Pull Request направляется в `main` и сливается
после одобрения преподавателя. Правила защиты ветки настраиваются в GitHub.
Существующие корневые файлы и лабораторные следует сохранить при переносе.

Публикация Releases запускается тегом `lab1-v*` после успешных проверок.
Коммит должен входить в `feature/lab1-dictionary-set` или `main`.
Для релиза из feature-ветки слияние в `main` не требуется:

```bash
git switch feature/lab1-dictionary-set
git pull --ff-only
git tag lab1-v1.0.0
git push origin lab1-v1.0.0
```

Удалённый запуск CI, PR и публикация Releases требуют переноса файлов в
репозиторий и здесь не заявляются выполненными.
