"""Require the intended static contract diagnostic, not just compilation failure."""
import subprocess
import sys

compiler, root, *selected = sys.argv[1:]
case = selected[0] if selected else "wrong_width"
cases = {
    "wrong_width": ("tests/wrong_width.cpp", ("connect", "deduced conflicting")),
    "logic_width": ("tests/compile_fail/logic_width.cpp", ("connect", "deduced conflicting")),
    "logic_role": ("tests/compile_fail/logic_role.cpp", ("connect", "no matching function")),
    "numeric_bit": ("tests/compile_fail/numeric_bit.cpp", ("Bit", "constraints not satisfied")),
    "custom_atom": ("tests/compile_fail/custom_atom.cpp", ("CircuitRoot", "constraints not satisfied")),
}
source, required = cases[case]
result = subprocess.run(
    [compiler, "-std=c++23", "-fsyntax-only", "-I" + root + "/include", root + "/" + source],
    capture_output=True, text=True)
if result.returncode == 0 or any(text not in result.stderr for text in required):
    sys.exit(f"Expected {case} diagnostic containing {required}:\n" + result.stderr)
