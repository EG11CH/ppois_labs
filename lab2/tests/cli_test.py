"""Smoke test the public executable, including validation and the rental lifecycle."""

from pathlib import Path
import subprocess
import sys


def run(program, commands):
    return subprocess.run(
        [str(program)],
        input=commands,
        capture_output=True,
        encoding="utf-8",
        check=True,
    ).stdout


def main():
    program = Path(sys.argv[1]).resolve()
    output = run(program, """
help
catalog
bad-command
available
available 101x 2026-02-05 2026-02-08
catalog extra
exit extra
available 101 2026-02-05 2026-02-08
customer-add 3 "Новый клиент" +375291234567 DEMO-3 2026-02-01 2030-01-01
customer-suspend 3
customer-restore 3
asset-add 103 1 SN-1 "Набор 1" Demo "Инструменты" 0 40000 1000 6000 2026-02-01
asset-retire 103 2026-02-02
catalog
quote 1 101 2026-02-01 2026-02-05 2026-02-08 -
reserve 1 101 2026-02-01 2026-02-05 2026-02-08 -
status
available 101 2026-02-05 2026-02-08
wait 2 101 2026-02-01 2026-02-05 2026-02-08 -
waiting 101 2026-02-02
move 1 2026-02-02 2026-02-08 2026-02-10 "Другие даты"
promote 1 2026-02-02
cancel 2 2026-02-02 "По просьбе"
expire 2026-02-02
issue 1 2026-02-08 15000 5 1 1 1 -
issue 1 2026-02-08 15000 1 1 1 0 "Неполный комплект"
issue 1 2026-02-08 15000 1 1 1 1 -
catalog
available 101 2026-02-10 2026-02-12
status
extend 1 2026-02-09 2026-02-11 "На день дольше"
return 1 1 2026-02-12 1 0 1 10000 "Трещина корпуса"
catalog
service-start 101 2026-02-12 "Замена корпуса"
service-finish 101 1 2026-02-13 2400
reserve 2 101 2026-02-13 2026-02-13 2026-02-14 -
issue 3 2026-02-13 15000 1 1 1 1 -
return 2 1 2026-02-14 1 1 1 0 -
status
exit
bad-command
""")
    for expected in (
        "Прокат инвентаря", "help | exit", "#101 DemoTool Дрель",
        "Неизвестная команда", "Не хватает аргументов", "Ожидается целое число",
        "Лишние аргументы", "Можно забронировать.", "Клиент #3 зарегистрирован.",
        "Клиент #3 заблокирован.", "Клиент #3 разблокирован.", "Предмет #103 добавлен.",
        "Предмет #103 списан.", "Аренда 71.25 BYN", "Бронь #1 действует",
        "Пересечение с бронью.", "Заявки ожидания: #1",
        "Бронь #1 перенесена.", "Создана бронь #2 из заявки #1.",
        "Бронь #2 отменена.", "Снято просроченных броней: 0",
        "Для флага укажи 0 или 1.", "Выдача требует осмотра без дефектов.",
        "Залог по договору #1: 150.00 BYN", "Предмет недоступен.",
        "Договор #1 действует", "Договор #1 продлён до 2026-02-11.",
        "итого 183.25 BYN, доплата 33.25 BYN", "Трещина корпуса",
        "пункт 1 | обслуживание", "открыт: Замена корпуса",
        "закрыт 2026-02-13", "возврат залога 125.00 BYN",
        "Бронь #1 выдано", "Договор #1 закрыт",
    ):
        if expected not in output:
            raise AssertionError(f"Missing {expected!r} in CLI output:\n{output}")
    if output.count("Неизвестная команда") != 1:
        raise AssertionError(f"Program processed input after exit:\n{output}")
    if run(program, "") != "Прокат инвентаря. Список команд: help\n":
        raise AssertionError("Program did not finish cleanly on EOF")


if __name__ == "__main__":
    main()
