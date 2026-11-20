"""Measure coverage of the lab2 model and CLI. Require more than 90% of executable lines."""

import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
MINIMUM_PERCENT = 90


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", nargs="?", default="build-lab2", type=Path)
    build_dir = parser.parse_args().build_dir.resolve()
    report = build_dir / "coverage-lab2"
    report.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            "gcovr", "--root", str(ROOT), "--filter", r"lab2/(include|src|cli)/",
            "--json-summary", str(report / "summary.json"),
            "--html-details", str(report / "index.html"), str(build_dir),
        ],
        cwd=ROOT, check=True,
    )
    summary = json.loads((report / "summary.json").read_text(encoding="utf-8"))
    covered, total = summary["line_covered"], summary["line_total"]
    if total == 0:
        raise SystemExit("No executable production lines were found")
    print(f"Lab2 line coverage: {covered / total:.2%} ({covered}/{total})")
    if covered * 100 <= MINIMUM_PERCENT * total:
        raise SystemExit(f"Lab2 coverage must be strictly greater than {MINIMUM_PERCENT}%")


if __name__ == "__main__":
    main()
