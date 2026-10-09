"""Package both standalone Linux programs, examples and generated documentation."""

import argparse
import shutil
import tarfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_directory")
    parser.add_argument("documentation_directory")
    parser.add_argument("output_directory", nargs="?", default="release")
    arguments = parser.parse_args()
    build_directory = Path(arguments.build_directory).resolve()
    documentation = Path(arguments.documentation_directory).resolve()
    output = Path(arguments.output_directory).resolve()
    package = output / "lab1-linux-x86_64"
    package.mkdir(parents=True, exist_ok=True)
    for executable in ("dictionary_cli", "set_cli"):
        shutil.copy2(build_directory / "lab1" / executable, package / executable)
    shutil.copytree(ROOT / "lab1" / "examples", package / "examples", dirs_exist_ok=True)
    shutil.copytree(documentation, package / "documentation", dirs_exist_ok=True)
    shutil.copy2(ROOT / "lab1" / "docs" / "RUNNING_BINARIES.md", package / "README.md")
    archive_path = output / "lab1-linux-x86_64.tar.gz"
    with tarfile.open(archive_path, "w:gz") as archive:
        archive.add(package, arcname=package.name)
    print(archive_path)


if __name__ == "__main__":
    main()
