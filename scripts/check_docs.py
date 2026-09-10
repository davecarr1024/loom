"""Check the design skeleton without claiming production-code verification."""

from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    required = [
        "README.md", "AGENTS.md", "docs/design.md", "docs/timing.md",
        "docs/roadmap.md", "docs/decisions.md", "docs/baseline.md", "docs/status.md",
    ]
    errors = []
    files = sorted(root.glob("*.md")) + sorted((root / "docs").glob("*.md"))
    for name in required:
        if not (root / name).is_file():
            errors.append(f"Missing required document: {name}")
    for path in files:
        body = path.read_text(encoding="utf-8")
        if not body.startswith("# ") or not body.endswith("\n"):
            errors.append(f"{path.relative_to(root)}: needs title and final newline")
        for number, line in enumerate(body.splitlines(), 1):
            if line.rstrip() != line:
                errors.append(f"{path.relative_to(root)}:{number}: trailing whitespace")
        for target in re.findall(r"\]\(([^)]+)\)", body):
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc:
                continue
            local = (path.parent / unquote(parsed.path)).resolve()
            if not local.is_relative_to(root) or not local.is_file():
                errors.append(f"{path.relative_to(root)}: invalid local link {target}")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"Documentation checks passed ({len(files)} Markdown files).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
