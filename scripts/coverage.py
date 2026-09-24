"""Enforce measured production line/function coverage from Bazel's LCOV report."""
from pathlib import Path
import sys

report = Path("bazel-out/_coverage/_coverage_report.dat")
if not report.is_file():
    sys.exit(
        "Missing Bazel LCOV report; run bazel coverage --combined_report=lcov //:circuit_tests"
    )

totals = {key: 0 for key in ("LF", "LH", "FNF", "FNH")}
source = None
production = False
line_hits = {}
excluded_lines = set()


def source_exclusions(path):
    """Read generic LCOV_EXCL_LINE annotations from a source file."""
    source_path = Path(path)
    if not source_path.is_file():
        sys.exit(f"Missing source file referenced by coverage report: {path}")
    return {
        number
        for number, text in enumerate(source_path.read_text(encoding="utf-8").splitlines(), 1)
        if "LCOV_EXCL_LINE" in text
    }


def finish_source():
    if not production:
        return
    for number, count in line_hits.items():
        if number not in excluded_lines:
            totals["LF"] += 1
            totals["LH"] += count > 0


for record in report.read_text(encoding="utf-8").splitlines():
    if record.startswith("SF:"):
        finish_source()
        source = record[3:]
        production = source.startswith("include/")
        line_hits = {}
        excluded_lines = source_exclusions(source) if production else set()
    elif production and record.startswith("DA:"):
        number, count = map(int, record[3:].split(",", 1))
        line_hits[number] = line_hits.get(number, 0) + count
    elif production and record.startswith("FNF:"):
        totals["FNF"] += int(record[4:])
    elif production and record.startswith("FNH:"):
        totals["FNH"] += int(record[4:])
finish_source()

if not totals["LF"] or not totals["FNF"]:
    sys.exit("No production coverage was collected from Bazel")
if totals["LF"] != totals["LH"] or totals["FNF"] != totals["FNH"]:
    sys.exit(f"Production coverage below 100%: {totals}")
print(f"100% measured production lines/functions: {totals}")
