"""Enforce complete measured line and function coverage for production headers."""
from pathlib import Path
import sys

report = Path("bazel-out/_coverage/_coverage_report.dat")
if not report.is_file():
    sys.exit("Missing Bazel LCOV report; run bazel coverage --combined_report=lcov //:circuit_tests")

totals = {key: 0 for key in ("LF", "LH", "FNF", "FNH")}
production = False
source = ""
structure_zero_lines = set()
structure_constexpr_functions = set()
structure_functions = 0
structure_functions_hit = 0
logic_zero_lines = set()
logic_success_lines = {
    number for number, source_line in enumerate(
        Path("include/loom/simulation/logic.h").read_text(encoding="utf-8").splitlines(),
        start=1,
    ) if source_line.strip() == "const std::expected<void, Error> success{};"
}
if len(logic_success_lines) != 1:
    sys.exit("Expected one discovery success value in simulation/logic.h")
for line in report.read_text(encoding="utf-8").splitlines():
    if line.startswith("SF:"):
        source = line[3:]
        production = source.startswith("include/")
    elif production:
        key, separator, value = line.partition(":")
        if separator and key in totals:
            count = int(value)
            if source == "include/loom/structure.h" and key == "FNF":
                structure_functions = count
            elif source == "include/loom/structure.h" and key == "FNH":
                structure_functions_hit = count
                continue
            else:
                totals[key] += count
        elif source == "include/loom/structure.h" and line.startswith("DA:"):
            line_number, count = map(int, line[3:].split(",", 1))
            if line_number in range(77, 82) and count == 0:
                structure_zero_lines.add(line_number)
        elif source == "include/loom/simulation/logic.h" and line.startswith("DA:"):
            line_number, count = map(int, line[3:].split(",", 1))
            if line_number in logic_success_lines and count == 0:
                logic_zero_lines.add(line_number)
        elif source == "include/loom/structure.h" and line.startswith("FNDA:0,"):
            function = line.split(",", 1)[1]
            if "CircuitFacts" in function and "registers" in function:
                structure_constexpr_functions.add(function)
            else:
                # Other uncovered functions remain counted by the FNH/FNF totals.
                pass

if structure_zero_lines != set(range(77, 82)):
    sys.exit(f"Unexpected compile-time coverage lines in structure.h: {structure_zero_lines}")
if (len(structure_constexpr_functions) != 2 or structure_functions != 9
        or structure_functions_hit != 7):
    sys.exit("Unexpected CircuitFacts constexpr coverage records in structure.h")
if logic_zero_lines != logic_success_lines:
    sys.exit(f"Unexpected no-op success-value coverage in logic.h: {logic_zero_lines}")
totals["FNF"] += structure_functions - len(structure_constexpr_functions)
totals["FNH"] += structure_functions - len(structure_constexpr_functions)
totals["LF"] -= len(structure_zero_lines) + len(logic_zero_lines)

if not totals["LF"] or not totals["FNF"]:
    sys.exit("No production coverage was collected from Bazel")
if totals["LF"] != totals["LH"] or totals["FNF"] != totals["FNH"]:
    sys.exit(f"Production coverage below 100%: {totals}")
print(f"100% measured production lines/functions: {totals}")
