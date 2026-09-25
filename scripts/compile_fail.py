"""Require the intended static contract diagnostic, not just compilation failure."""
import subprocess
import sys
import os
from pathlib import Path

case = sys.argv[1] if len(sys.argv) > 1 else "wrong_width"
root = Path(os.environ["TEST_SRCDIR"]) / os.environ["TEST_WORKSPACE"]
compiler = os.environ.get("CXX", "clang++-19")
cases = {
    "wrong_width": ("tests/wrong_width.cpp", ("connect", "deduced conflicting")),
    "logic_width": ("tests/compile_fail/logic_width.cpp", ("connect", "deduced conflicting")),
    "logic_role": ("tests/compile_fail/logic_role.cpp", ("connect", "no matching function")),
    "numeric_bit": ("tests/compile_fail/numeric_bit.cpp", ("Bit", "constraints not satisfied")),
    "custom_atom": ("tests/compile_fail/custom_atom.cpp", ("CircuitRoot", "constraints not satisfied")),
    "bundle_width": ("tests/compile_fail/bundle_width.cpp", ("connect", "no matching function")),
    "bundle_role": ("tests/compile_fail/bundle_role.cpp", ("connect", "no matching function")),
    "mux_word_zero_width": ("tests/compile_fail/mux_word_zero_width.cpp", ("word mux width must be positive",)),
    "word_register_zero_width": ("tests/compile_fail/word_register_zero_width.cpp", ("word register width must be positive",)),
}
source, required = cases[case]
result = subprocess.run(
    [compiler, "-std=c++23", "-fsyntax-only", "-I" + str(root / "include"), str(root / source)],
    capture_output=True, text=True)
if result.returncode == 0 or any(text not in result.stderr for text in required):
    sys.exit(f"Expected {case} diagnostic containing {required}:\n" + result.stderr)
