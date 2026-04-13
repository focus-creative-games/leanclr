#!/usr/bin/env python3
"""
Scan LeanCLR runtime icalls/ and intrinsics/ C++ sources for registration tables
and emit:
  - icalls.json
  - intrinsics.json
  - icalls_newobj.json
  - intrinsics_newobj.json

  [ {
      "name": "<managed icall/intrinsic name>",
      "func": "<C++ Class::method>",
      "header": "<path relative to src/runtime, e.g. icalls/foo.h>"
    }, ... ]

Every entry must have a non-empty `func`. If an `InternalCallEntry` still uses
`nullptr` for the function pointer, this script aborts with `RuntimeError`.

Default layout: this file lives in src/generator; repository root is three levels up.
JSON files are written to src/leanaot/LeanAOT/ by default.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


def _default_repo_root() -> Path:
    """``src/generator/this.py`` -> repository root (parent of ``src``)."""
    return Path(__file__).resolve().parent.parent.parent


def strip_cpp_comments(text: str) -> str:
    """Remove // and /* */ comments (not handling raw strings)."""
    out: list[str] = []
    i = 0
    n = len(text)
    in_line_comment = False
    in_block_comment = False
    in_string = False
    in_char = False
    escape = False

    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if in_line_comment:
            if ch == "\n":
                in_line_comment = False
                out.append(ch)
            i += 1
            continue

        if in_block_comment:
            if ch == "*" and nxt == "/":
                in_block_comment = False
                i += 2
            else:
                i += 1
            continue

        if in_string:
            out.append(ch)
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == '"':
                in_string = False
            i += 1
            continue

        if in_char:
            out.append(ch)
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == "'":
                in_char = False
            i += 1
            continue

        if ch == "/" and nxt == "/":
            in_line_comment = True
            i += 2
            continue
        if ch == "/" and nxt == "*":
            in_block_comment = True
            i += 2
            continue

        if ch == '"':
            in_string = True
            out.append(ch)
            i += 1
            continue
        if ch == "'":
            in_char = True
            out.append(ch)
            i += 1
            continue

        out.append(ch)
        i += 1

    return "".join(out)


def decode_c_string_literal(inner: str) -> str:
    """Decode contents of a C/C++ string literal (after opening quote)."""
    i = 0
    parts: list[str] = []
    while i < len(inner):
        c = inner[i]
        if c != "\\":
            parts.append(c)
            i += 1
            continue
        if i + 1 >= len(inner):
            parts.append("\\")
            break
        esc = inner[i + 1]
        i += 2
        if esc == "n":
            parts.append("\n")
        elif esc == "t":
            parts.append("\t")
        elif esc == "r":
            parts.append("\r")
        elif esc == '"':
            parts.append('"')
        elif esc == "'":
            parts.append("'")
        elif esc == "\\":
            parts.append("\\")
        elif esc == "0":
            parts.append("\0")
        elif esc == "x" and i < len(inner):
            # \xHH...
            j = i
            while j < len(inner) and inner[j] in "0123456789abcdefABCDEF":
                j += 1
            hex_digits = inner[i:j]
            i = j
            try:
                parts.append(chr(int(hex_digits, 16)))
            except ValueError:
                parts.append("\\x" + hex_digits)
        elif esc in "01234567":
            # octal up to 3 digits
            digits = esc
            for _ in range(2):
                if i < len(inner) and inner[i] in "01234567":
                    digits += inner[i]
                    i += 1
                else:
                    break
            try:
                parts.append(chr(int(digits, 8)))
            except ValueError:
                parts.append("\\" + digits)
        else:
            parts.append(esc)
    return "".join(parts)


# Adjacent C++ string literals (optionally split across lines), e.g. "a" "b"
_CSTRING_LITERAL_CHUNK = r'"(?:[^"\\]|\\.)*"'


def parse_concatenated_c_string_literals(blob: str) -> str:
    """Join one or more adjacent \"...\" chunks (as in C++ source)."""
    pieces = re.findall(_CSTRING_LITERAL_CHUNK, blob)
    if not pieces:
        return ""
    inner = [p[1:-1] for p in pieces]
    return "".join(decode_c_string_literal(x) for x in inner)


# First field: one or more string literals; second: (..InternalCallFunction..) & Class::method
_ICALL_FP_RE = re.compile(
    rf"\{{\s*((?:{_CSTRING_LITERAL_CHUNK}\s*)+),\s*\([^)]*InternalCallFunction\)\s*&?\s*"
    r"([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*,",
    re.DOTALL,
)

# Detect legacy invoker-only rows (must not appear; func is required in JSON)
_ICALL_NULLPTR_RE = re.compile(
    rf"\{{\s*((?:{_CSTRING_LITERAL_CHUNK}\s*)+),\s*nullptr\s*,",
    re.DOTALL,
)

_INTRINSIC_ENTRY_RE = re.compile(
    rf"\{{\s*((?:{_CSTRING_LITERAL_CHUNK}\s*)+),\s*\([^)]*IntrinsicFunction\)\s*&?\s*"
    r"([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*,",
    re.DOTALL,
)

_ICALL_NEWOBJ_ENTRY_RE = re.compile(
    rf"\{{\s*((?:{_CSTRING_LITERAL_CHUNK}\s*)+),\s*([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\}}",
    re.DOTALL,
)

_INTRINSIC_NEWOBJ_ENTRY_RE = re.compile(
    rf"\{{\s*((?:{_CSTRING_LITERAL_CHUNK}\s*)+),\s*([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\}}",
    re.DOTALL,
)


def cpp_to_runtime_header_rel(cpp_path: Path, runtime_root: Path) -> str:
    """Path to sibling .h under src/runtime, posix, e.g. icalls/system_threading_volatile.h."""
    rel = cpp_path.resolve().relative_to(runtime_root.resolve())
    return rel.with_suffix(".h").as_posix()


def invoker_to_newobj_func(invoker_name: str, source: Path) -> str:
    if not invoker_name.endswith("_invoker"):
        raise RuntimeError(f"{source}: newobj invoker must end with '_invoker', got {invoker_name!r}")
    return invoker_name[: -len("_invoker")]


def collect_intrinsics_from_file(cpp_path: Path, runtime_root: Path) -> list[tuple[str, str, str, str]]:
    """Returns list of (name, func, source_relpath, header_rel_runtime)."""
    text = cpp_path.read_text(encoding="utf-8", errors="replace")
    cleaned = strip_cpp_comments(text)
    rel = str(cpp_path).replace("\\", "/")
    header_rel = cpp_to_runtime_header_rel(cpp_path, runtime_root)
    rows: list[tuple[str, str, str, str]] = []
    for m in _INTRINSIC_ENTRY_RE.finditer(cleaned):
        name = parse_concatenated_c_string_literals(m.group(1))
        rows.append((name, m.group(2), rel, header_rel))
    return rows


def collect_intrinsic_newobj_from_file(cpp_path: Path, runtime_root: Path) -> list[tuple[str, str, str, str]]:
    text = cpp_path.read_text(encoding="utf-8", errors="replace")
    cleaned = strip_cpp_comments(text)
    rel = str(cpp_path).replace("\\", "/")
    header_rel = cpp_to_runtime_header_rel(cpp_path, runtime_root)
    rows: list[tuple[str, str, str, str]] = []
    for m in _INTRINSIC_NEWOBJ_ENTRY_RE.finditer(cleaned):
        name = parse_concatenated_c_string_literals(m.group(1))
        invoker = m.group(2)
        if ".ctor(" not in name:
            continue
        rows.append((name, invoker_to_newobj_func(invoker, cpp_path), rel, header_rel))
    return rows


def collect_icalls_from_file(cpp_path: Path, runtime_root: Path) -> list[tuple[str, str, str, str]]:
    """Returns list of (name, func, source_relpath, header_rel_runtime)."""
    text = cpp_path.read_text(encoding="utf-8", errors="replace")
    cleaned = strip_cpp_comments(text)
    rel = str(cpp_path).replace("\\", "/")

    nullptr_names = [
        parse_concatenated_c_string_literals(m.group(1))
        for m in _ICALL_NULLPTR_RE.finditer(cleaned)
    ]
    if nullptr_names:
        preview = "; ".join(repr(n) for n in nullptr_names[:8])
        more = f" (+{len(nullptr_names) - 8} more)" if len(nullptr_names) > 8 else ""
        raise RuntimeError(
            f"{cpp_path}: InternalCallEntry uses nullptr for func ({len(nullptr_names)}): {preview}{more}"
        )

    header_rel = cpp_to_runtime_header_rel(cpp_path, runtime_root)
    rows: list[tuple[str, str, str, str]] = []
    for m in _ICALL_FP_RE.finditer(cleaned):
        name = parse_concatenated_c_string_literals(m.group(1))
        rows.append((name, m.group(2), rel, header_rel))
    return rows


def collect_icall_newobj_from_file(cpp_path: Path, runtime_root: Path) -> list[tuple[str, str, str, str]]:
    text = cpp_path.read_text(encoding="utf-8", errors="replace")
    cleaned = strip_cpp_comments(text)
    rel = str(cpp_path).replace("\\", "/")
    header_rel = cpp_to_runtime_header_rel(cpp_path, runtime_root)
    rows: list[tuple[str, str, str, str]] = []
    for m in _ICALL_NEWOBJ_ENTRY_RE.finditer(cleaned):
        name = parse_concatenated_c_string_literals(m.group(1))
        invoker = m.group(2)
        if ".ctor(" not in name:
            continue
        rows.append((name, invoker_to_newobj_func(invoker, cpp_path), rel, header_rel))
    return rows


def merge_entries(
    label: str,
    rows: list[tuple[str, str, str, str]],
    runtime_root: Path,
) -> list[dict[str, str]]:
    """Deduplicate by name; warn on conflicting func or header."""
    by_name: dict[str, tuple[str, str, str]] = {}
    for name, func, src, header in rows:
        if not func or not func.strip():
            raise RuntimeError(f"{src}: empty func for {label} name {name!r}")
        if not (runtime_root / header).is_file():
            print(f"warning: header missing for {label} entry from {src}: {header}", file=sys.stderr)

        if name not in by_name:
            by_name[name] = (func, src, header)
            continue
        old_func, old_src, old_header = by_name[name]
        if old_func != func:
            print(
                f"warning: duplicate {label} name {name!r} with different func: "
                f"{old_func!r} ({old_src}) vs {func!r} ({src})",
                file=sys.stderr,
            )
        elif old_header != header:
            print(
                f"warning: duplicate {label} name {name!r} with different header: "
                f"{old_header!r} ({old_src}) vs {header!r} ({src})",
                file=sys.stderr,
            )
    ordered = sorted(by_name.keys(), key=lambda s: s.lower())
    return [{"name": n, "func": by_name[n][0], "header": by_name[n][2]} for n in ordered]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--repo-root",
        type=Path,
        default=_default_repo_root(),
        help="Repository root (default: parent of src/, inferred from this script location)",
    )
    ap.add_argument(
        "--icalls-dir",
        type=Path,
        default=None,
        help="Override icalls directory (default: <repo-root>/src/runtime/icalls)",
    )
    ap.add_argument(
        "--intrinsics-dir",
        type=Path,
        default=None,
        help="Override intrinsics directory (default: <repo-root>/src/runtime/intrinsics)",
    )
    ap.add_argument(
        "--out-dir",
        type=Path,
        default=None,
        help="Output directory for JSON files (default: <repo-root>/src/leanaot/LeanAOT)",
    )
    args = ap.parse_args()

    repo = args.repo_root.resolve()
    icalls_dir = (args.icalls_dir or repo / "src" / "runtime" / "icalls").resolve()
    intr_dir = (args.intrinsics_dir or repo / "src" / "runtime" / "intrinsics").resolve()
    out_dir = (args.out_dir or repo / "src" / "leanaot" / "LeanAOT").resolve()

    if not icalls_dir.is_dir():
        print(f"error: icalls directory not found: {icalls_dir}", file=sys.stderr)
        return 1
    if not intr_dir.is_dir():
        print(f"error: intrinsics directory not found: {intr_dir}", file=sys.stderr)
        return 1

    runtime_root = icalls_dir.parent.resolve()
    if intr_dir.parent.resolve() != runtime_root:
        print(
            f"error: icalls and intrinsics must be sibling folders under the same runtime root "
            f"(expected intrinsics under {runtime_root}, got {intr_dir.parent.resolve()})",
            file=sys.stderr,
        )
        return 1

    try:
        icall_rows: list[tuple[str, str, str, str]] = []
        intrinsic_rows: list[tuple[str, str, str, str]] = []
        icall_newobj_rows: list[tuple[str, str, str, str]] = []
        intrinsic_newobj_rows: list[tuple[str, str, str, str]] = []

        for cpp in sorted(icalls_dir.glob("*.cpp")):
            icall_rows.extend(collect_icalls_from_file(cpp, runtime_root))
            icall_newobj_rows.extend(collect_icall_newobj_from_file(cpp, runtime_root))

        for cpp in sorted(intr_dir.glob("*.cpp")):
            intrinsic_rows.extend(collect_intrinsics_from_file(cpp, runtime_root))
            intrinsic_newobj_rows.extend(collect_intrinsic_newobj_from_file(cpp, runtime_root))

        icalls_json = merge_entries("icall", icall_rows, runtime_root)
        intrinsics_json = merge_entries("intrinsic", intrinsic_rows, runtime_root)
        icall_newobj_json = merge_entries("icall_newobj", icall_newobj_rows, runtime_root)
        intrinsic_newobj_json = merge_entries("intrinsic_newobj", intrinsic_newobj_rows, runtime_root)
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "icalls.json").write_text(
        json.dumps(icalls_json, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    (out_dir / "intrinsics.json").write_text(
        json.dumps(intrinsics_json, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    (out_dir / "icalls_newobj.json").write_text(
        json.dumps(icall_newobj_json, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    (out_dir / "intrinsics_newobj.json").write_text(
        json.dumps(intrinsic_newobj_json, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print(
        f"Wrote {len(icalls_json)} icalls -> {out_dir / 'icalls.json'}\n"
        f"Wrote {len(intrinsics_json)} intrinsics -> {out_dir / 'intrinsics.json'}\n"
        f"Wrote {len(icall_newobj_json)} icall newobj entries -> {out_dir / 'icalls_newobj.json'}\n"
        f"Wrote {len(intrinsic_newobj_json)} intrinsic newobj entries -> {out_dir / 'intrinsics_newobj.json'}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
