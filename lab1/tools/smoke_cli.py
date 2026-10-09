"""Run the actual entry points with a short, meaningful interaction."""

import subprocess
import sys

scenarios = (
    (sys.argv[1], "1\ncat\nкот\n3\ncat\n0\n", "Перевод: кот"),
    (sys.argv[2], "1\n{a, b}\n11\n0\n", "Число подмножеств: 4"),
)
for executable, commands, expected in scenarios:
    result = subprocess.run(
        [executable], input=commands, text=True, capture_output=True, timeout=10, check=True
    )
    if expected not in result.stdout:
        raise RuntimeError(f"Unexpected CLI output: {result.stdout}")
print("Both executable entry points passed")
