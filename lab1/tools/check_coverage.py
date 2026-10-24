"""Measure production line coverage and require strictly more than 90 percent."""

import argparse
import json
import subprocess
from pathlib import Path

MINIMUM_COVERAGE = 90.0
ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_directory", nargs="?", default="build")
    arguments = parser.parse_args()
    build_directory = Path(arguments.build_directory).resolve()
    reports = build_directory / "coverage"
    reports.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            "gcovr", "--root", str(ROOT), "--filter", "lab1/(include|src|cli)/",
            "--json-summary", str(reports / "summary.json"),
            "--html-details", str(reports / "index.html"), str(build_directory),
        ],
        cwd=ROOT, check=True,
    )
    summary = json.loads((reports / "summary.json").read_text(encoding="utf-8"))
    total = summary["line_percent"]
    print(f"Production line coverage: {total:.2f}%")
    if summary["line_total"] == 0 or total <= MINIMUM_COVERAGE:
        raise SystemExit("Required: strictly more than 90% line coverage")


if __name__ == "__main__":
    main()
