"""Run compatible clang-tidy, with the explicit Rule Lab C++23 exception."""
import re
import subprocess
import os
from pathlib import Path

version = subprocess.check_output(["clang-tidy-19", "--version"], text=True)
major = re.search(r"LLVM version (\d+)", version)
if major and int(major.group(1)) < 16:
    print("SKIPPED clang-tidy: frontend older than 16 cannot parse std::expected")
else:
    root = Path(os.environ["TEST_SRCDIR"]) / os.environ["TEST_WORKSPACE"]
    sources = sorted(str(path.relative_to(root)) for path in
                     (root / "examples").glob("*.cpp"))
    subprocess.run(["clang-tidy-19", "--warnings-as-errors=*",
                    "--extra-arg=-std=c++23", f"--extra-arg=-I{root / 'include'}",
                    *[str(root / source) for source in sources]], check=True)
