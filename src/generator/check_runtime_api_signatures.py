#!/usr/bin/env python3
"""Verify icalls.json / intrinsics.json names against externs.txt signatures."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

LINE_RE = re.compile(r"^\[[^\]]+\]\s+(.+)$")
# DeclaringType::MemberName with optional (params) at end of dnlib FullName tail.
MEMBER_SIG_RE = re.compile(
    r"((?:[\w/`\.`][\w/`\.`<>,*&\[\]+\-]*)::\.?[\w`<>]+)(\([^)]*\))?\s*$"
)

# Extra LeanCLR implementations not listed in externs.txt (exact `name` field match).
ICALLS_WHITELIST: frozenset[str] = frozenset(
    {
        "System.Threading.Interlocked::CompareExchange(System.Object&,System.Object,System.Object)",
    }
)

INTRINSICS_WHITELIST: frozenset[str] = frozenset(
    {
        "System.Array::get_Length",
        "System.Array::get_LongLength",
        "System.Array::GetGenericValueImpl<>",
        "System.Array::SetGenericValueImpl<>",
        "System.Object::.ctor()",
        "System.String::get_Length",
        "System.String::GetHashCode",
        "System.String::GetLegacyNonRandomizedHashCode",
        "System.Threading.Interlocked::Exchange(System.Object&,System.Object)",
        "System.Threading.Interlocked::MemoryBarrier",
    }
)


def member_signature_from_extern(rest: str) -> str | None:
    match = MEMBER_SIG_RE.search(rest.strip())
    if not match:
        return None
    return match.group(1) + (match.group(2) or "")


def is_generic_member_sig(member_sig: str) -> bool:
    """True when the member name contains generic arity, e.g. Write<T> or Exchange<>."""
    member_name = member_sig.split("(", 1)[0]
    return "<" in member_name and ">" in member_name


def normalize_generic_member_sig(member_sig: str) -> str:
    """Map CompareExchange<T>(T&,T,T) -> System.Threading.Interlocked::CompareExchange<>."""
    member_name = member_sig.split("(", 1)[0]
    return re.sub(r"<[^>]*>", "<>", member_name)


def canonical_lookup_keys(name: str) -> list[str]:
    """Keys used to match a JSON `name` against the extern signature set."""
    if is_generic_member_sig(name):
        return [normalize_generic_member_sig(name)]
    return [name, name.split("(", 1)[0]]


def add_extern_signatures(signature_set: set[str], member_sig: str) -> None:
    if is_generic_member_sig(member_sig):
        signature_set.add(normalize_generic_member_sig(member_sig))
        return
    signature_set.add(member_sig)
    signature_set.add(member_sig.split("(", 1)[0])


def load_extern_signatures(paths: list[Path]) -> tuple[set[str], int]:
    signatures: set[str] = set()
    line_count = 0
    for path in paths:
        text = path.read_text(encoding="utf-8-sig")
        for raw_line in text.splitlines():
            line = raw_line.strip()
            if not line:
                continue
            match = LINE_RE.match(line)
            if not match:
                continue
            member_sig = member_signature_from_extern(match.group(1))
            if not member_sig:
                continue
            add_extern_signatures(signatures, member_sig)
            line_count += 1
    return signatures, line_count


def load_json_entries(path: Path) -> list[dict]:
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    if not isinstance(data, list):
        raise ValueError(f"Expected JSON array in {path}")
    return data


def is_whitelisted(json_kind: str, name: str) -> bool:
    if json_kind == "icalls":
        return name in ICALLS_WHITELIST
    if json_kind == "intrinsics":
        return name in INTRINSICS_WHITELIST
    return False


def name_matches_signatures(name: str, signatures: set[str]) -> bool:
    return any(key in signatures for key in canonical_lookup_keys(name))


def check_json_file(path: Path, signatures: set[str], json_kind: str) -> list[dict]:
    missing = []
    for entry in load_json_entries(path):
        name = entry.get("name")
        if not isinstance(name, str) or not name.strip():
            missing.append({"file": path.name, "name": name, "func": entry.get("func"), "reason": "empty name"})
            continue
        if is_whitelisted(json_kind, name):
            continue
        if not name_matches_signatures(name, signatures):
            missing.append({"file": path.name, "name": name, "func": entry.get("func"), "reason": "not in externs"})
    return missing


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Check icalls.json / intrinsics.json name fields against externs.txt. "
            "Non-generic extern entries contribute signatures with and without parameters. "
            "Generic extern entries contribute a single signature: Type::Method<> (no parameters)."
        )
    )
    parser.add_argument(
        "--externs",
        action="append",
        required=True,
        type=Path,
        metavar="FILE",
        help="externs.txt path; repeat to merge multiple files (e.g. win + linux mscorlib).",
    )
    parser.add_argument(
        "--icalls",
        action="append",
        type=Path,
        default=[],
        metavar="FILE",
        help="icalls.json path; repeat for multiple files.",
    )
    parser.add_argument(
        "--intrinsics",
        action="append",
        type=Path,
        default=[],
        metavar="FILE",
        help="intrinsics.json path; repeat for multiple files.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    extern_paths = [p.resolve() for p in args.externs]
    for path in extern_paths:
        if not path.is_file():
            print(f"error: externs file not found: {path}", file=sys.stderr)
            return 2

    signatures, extern_count = load_extern_signatures(extern_paths)
    print(f"Loaded {extern_count} extern entries from {len(extern_paths)} file(s) -> {len(signatures)} signatures")

    json_paths: list[tuple[str, Path]] = []
    for path in args.icalls:
        json_paths.append(("icalls", path.resolve()))
    for path in args.intrinsics:
        json_paths.append(("intrinsics", path.resolve()))

    if not json_paths:
        print("error: specify at least one --icalls or --intrinsics file", file=sys.stderr)
        return 2

    all_missing: list[dict] = []
    for kind, path in json_paths:
        if not path.is_file():
            print(f"error: {kind} file not found: {path}", file=sys.stderr)
            return 2
        entries = load_json_entries(path)
        missing = check_json_file(path, signatures, kind)
        whitelisted_count = sum(1 for e in entries if isinstance(e.get("name"), str) and is_whitelisted(kind, e["name"]))
        print(
            f"{path.name}: {len(entries)} entries, {len(missing)} not in externs"
            + (f" ({whitelisted_count} whitelisted)" if whitelisted_count else "")
        )
        all_missing.extend(missing)

    if all_missing:
        print("\nMissing entries:")
        for item in all_missing:
            func = item.get("func")
            func_suffix = f"  func={func}" if func else ""
            print(f"  [{item['file']}] {item['name']}{func_suffix}")
        return 1

    print("All entries matched extern signatures.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
