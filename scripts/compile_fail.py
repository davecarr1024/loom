"""Require a width-mismatch diagnostic, not just an unsuccessful compilation."""
import subprocess
import sys

compiler, root = sys.argv[1:]
result = subprocess.run(
    [compiler, "-std=c++23", "-fsyntax-only", "-I" + root + "/include",
     root + "/tests/wrong_width.cpp"], capture_output=True, text=True)
if result.returncode == 0 or "connect" not in result.stderr or "deduced conflicting" not in result.stderr:
    sys.exit("Expected a conflicting connect width diagnostic:\n" + result.stderr)
