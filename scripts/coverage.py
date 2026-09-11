"""Enforce measured production line/function coverage without excluding paths."""
from pathlib import Path
import os
import subprocess
import sys

build = Path("build-coverage")
subprocess.run(["lcov", "--zerocounters", "--directory", str(build)], check=True)
subprocess.run(["ctest", "--test-dir", str(build), "--output-on-failure"], check=True)
subprocess.run(["lcov", "--capture", "--gcov-tool", os.environ.get("GCOV", "gcov"),
                "--directory", str(build), "--output-file",
                str(build / "coverage.info")], check=True)
subprocess.run(["lcov", "--extract", str(build / "coverage.info"),
                str(Path("include").resolve()) + "/*", "--output-file",
                str(build / "production.info")], check=True)
totals = {key: 0 for key in ("LF", "LH", "FNF", "FNH")}
for line in (build / "production.info").read_text().splitlines():
    key, _, value = line.partition(":")
    if key in totals:
        totals[key] += int(value)
if not totals["LF"] or not totals["FNF"]:
    sys.exit("No production coverage was collected")
if totals["LF"] != totals["LH"] or totals["FNF"] != totals["FNH"]:
    sys.exit(f"Production coverage below 100%: {totals}")
print(f"100% measured production lines/functions: {totals}")
