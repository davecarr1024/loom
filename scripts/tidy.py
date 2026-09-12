"""Run compatible clang-tidy, with the explicit Rule Lab C++23 exception."""
import re
import subprocess

version = subprocess.check_output(["clang-tidy", "--version"], text=True)
major = re.search(r"LLVM version (\d+)", version)
if major and int(major.group(1)) < 16:
    print("SKIPPED clang-tidy: frontend older than 16 cannot parse std::expected")
else:
    subprocess.run(["clang-tidy", "--warnings-as-errors=*", "-p", "build",
                    "tests/circuit_test.cpp", "tests/not_test.cpp", "examples/transfer.cpp", "examples/not.cpp"], check=True)
