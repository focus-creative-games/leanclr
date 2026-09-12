#!/usr/bin/env python3
"""Verify runtime API JSON names against externs.txt signatures."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

LINE_RE = re.compile(r"^\[[^\]]+\]\s+(.+)$")
# DeclaringType::MemberName with optional (params) at end of dnlib FullName tail.
MEMBER_SIG_RE = re.compile(
    r"((?:[\w/`\.`][\w/`\.`<>,*&\[\]+\-|]*)::\.?[\w`<>|]+)(\(.*\))?\s*$"
)

# Extra LeanCLR implementations not listed in externs.txt (exact `name` field match).
ICALLS_WHITELIST: frozenset[str] = frozenset(
    {
        "System.RuntimeTypeHandle::IsByRef(System.RuntimeType)",
        "System.RuntimeTypeHandle::IsPointer(System.RuntimeType)",
        "System.RuntimeTypeHandle::IsPrimitive(System.RuntimeType)",
        "System.Threading.Interlocked::CompareExchange(System.Object&,System.Object,System.Object)",
    }
)

INTRINSICS_WHITELIST: frozenset[str] = frozenset(
    {
        "System.Array::get_Length",
        "System.Array::get_LongLength",
        "System.Array::Copy(System.Array,System.Array,System.Int32)",
        "System.Array::Copy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)",
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


FORBIDDEN_PROFILE_PATTERNS: dict[str, tuple[tuple[re.Pattern[str], str], ...]] = {
    "coreclr-net10": (
        (re.compile(r"\bMono\."), "Mono namespace entry"),
        (re.compile(r"\bSystem\.Mono"), "Mono custom attribute/runtime entry"),
        (re.compile(r"\bSystem\.IO\.Mono"), "Mono IO entry"),
        (re.compile(r"\bSystem\.Reflection\.Mono"), "Mono reflection entry"),
        (re.compile(r"\bSystem\.Runtime\.Remoting"), "remoting entry"),
        (re.compile(r"\bmscorlib\b", re.IGNORECASE), "mscorlib-era entry"),
        (re.compile(r"\bSystemMono[A-Za-z0-9_]*"), "Mono-era implementation symbol"),
        (re.compile(r"\bMono[A-Za-z0-9_]*::"), "Mono-era implementation symbol"),
        (re.compile(r"(?:^|[/\\])mono_[^/\\]*", re.IGNORECASE), "Mono-era runtime header"),
    ),
}


def forbidden_profile_reason(profile: str | None, entry: dict) -> str | None:
    if not profile:
        return None
    rules = FORBIDDEN_PROFILE_PATTERNS.get(profile)
    if not rules:
        return None

    entry_text = " ".join(
        str(value)
        for key, value in entry.items()
        if key in ("name", "func", "header") and isinstance(value, str)
    )
    for pattern, reason in rules:
        if pattern.search(entry_text):
            return reason
    return None


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


def load_pinvoke_contract_entries(path: Path) -> list[dict]:
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    if isinstance(data, list):
        return data
    if not isinstance(data, dict):
        raise ValueError(f"Expected JSON object or array in {path}")

    names = data.get("runtimeApiNames", [])
    if names is None:
        return []
    if not isinstance(names, list):
        raise ValueError(f"Expected runtimeApiNames array in {path}")

    entries: list[dict] = []
    for item in names:
        if isinstance(item, str):
            entries.append({"name": item})
        elif isinstance(item, dict):
            entries.append(item)
        else:
            entries.append({"name": item})
    return entries


def load_runtime_api_entries(path: Path, json_kind: str) -> list[dict]:
    if json_kind == "pinvokes":
        return load_pinvoke_contract_entries(path)
    return load_json_entries(path)


def is_whitelisted(json_kind: str, name: str) -> bool:
    if json_kind == "icalls":
        return name in ICALLS_WHITELIST
    if json_kind == "intrinsics":
        return name in INTRINSICS_WHITELIST
    return False


def name_matches_signatures(name: str, signatures: set[str]) -> bool:
    return any(key in signatures for key in canonical_lookup_keys(name))


def check_json_file(path: Path, signatures: set[str], json_kind: str, profile: str | None = None) -> list[dict]:
    missing = []
    for entry in load_runtime_api_entries(path, json_kind):
        name = entry.get("name")
        if not isinstance(name, str) or not name.strip():
            missing.append({"file": path.name, "name": name, "func": entry.get("func"), "reason": "empty name"})
            continue
        forbidden_reason = forbidden_profile_reason(profile, entry)
        if forbidden_reason:
            missing.append(
                {
                    "file": path.name,
                    "name": name,
                    "func": entry.get("func"),
                    "reason": f"forbidden in {profile}: {forbidden_reason}",
                }
            )
            continue
        if is_whitelisted(json_kind, name):
            continue
        if not name_matches_signatures(name, signatures):
            missing.append({"file": path.name, "name": name, "func": entry.get("func"), "reason": "not in externs"})
    return missing


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Check runtime API JSON name fields against externs.txt. "
            "Non-generic extern entries contribute signatures with and without parameters. "
            "Generic extern entries contribute a single signature: Type::Method<> (no parameters)."
        )
    )
    parser.add_argument(
        "--profile",
        choices=("coreclr-net10",),
        help="Runtime API profile. coreclr-net10 uses artifacts/dotnet10-externs.",
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        help="Repository root. Defaults to the parent of src/.",
    )
    parser.add_argument(
        "--runtime-api-dir",
        type=Path,
        help="Directory containing icalls.json and intrinsics.json. If omitted with --profile, uses src/leanaot/LeanAOT/runtime-apis/<profile>.",
    )
    parser.add_argument(
        "--externs-dir",
        type=Path,
        help="Directory containing *_externs.txt files. If supplied, all matching files are merged.",
    )
    parser.add_argument(
        "--externs",
        action="append",
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
    parser.add_argument(
        "--pinvokes",
        action="append",
        type=Path,
        default=[],
        metavar="FILE",
        help="pinvokes.json path; repeat for multiple files. Object-form pinvokes.json may expose contract names via runtimeApiNames.",
    )
    parser.add_argument(
        "--diff-report",
        type=Path,
        help="Write a JSON diff report with missing_from_runtime_api, extra_runtime_api, signature_changed, and renamed sections.",
    )
    parser.add_argument(
        "--fail-on-missing-externs",
        action="store_true",
        help="Return failure when extern signatures are not covered by runtime API JSON. By default the report is informational.",
    )
    return parser.parse_args()


def default_repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def collect_extern_paths(args: argparse.Namespace, repo_root: Path) -> list[Path]:
    extern_paths = list(args.externs or [])

    externs_dir = args.externs_dir
    if args.profile and not extern_paths and externs_dir is None:
        if args.profile == "coreclr-net10":
            externs_dir = repo_root / "artifacts" / "dotnet10-externs"

    if externs_dir is not None:
        if not externs_dir.is_dir():
            raise FileNotFoundError(f"externs directory not found: {externs_dir}")
        extern_paths.extend(sorted(externs_dir.glob("*_externs.txt")))

    return extern_paths


def resolve_runtime_api_dir(args: argparse.Namespace, repo_root: Path) -> Path | None:
    if args.runtime_api_dir:
        return args.runtime_api_dir

    if not args.profile:
        return None

    profile_dir = repo_root / "src" / "leanaot" / "LeanAOT" / "runtime-apis" / args.profile
    if profile_dir.is_dir():
        return profile_dir

    return profile_dir


def collect_runtime_api_lookup_keys(json_paths: list[tuple[str, Path]]) -> set[str]:
    lookup_keys: set[str] = set()
    for _kind, path in json_paths:
        if not path.is_file():
            continue
        for entry in load_runtime_api_entries(path, _kind):
            name = entry.get("name")
            if isinstance(name, str) and name.strip():
                lookup_keys.update(canonical_lookup_keys(name))
    return lookup_keys


def collect_diff_report_runtime_api_paths(
    json_paths: list[tuple[str, Path]], runtime_api_dir: Path | None
) -> list[tuple[str, Path]]:
    report_paths = list(json_paths)
    seen = {path.resolve() for _kind, path in report_paths}

    if runtime_api_dir is None:
        return report_paths

    for kind, file_name in (
        ("icalls_newobj", "icalls_newobj.json"),
        ("intrinsics_newobj", "intrinsics_newobj.json"),
        ("pinvokes", "pinvokes.json"),
    ):
        path = (runtime_api_dir / file_name).resolve()
        if path.is_file() and path not in seen:
            report_paths.append((kind, path))
            seen.add(path)

    return report_paths


def write_diff_report(
    report_path: Path,
    extern_paths: list[Path],
    json_paths: list[tuple[str, Path]],
    signatures: set[str],
    extra_runtime_api: list[dict],
) -> list[str]:
    runtime_api_keys = collect_runtime_api_lookup_keys(json_paths)
    missing_from_runtime_api = sorted(signature for signature in signatures if signature not in runtime_api_keys)

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "externFiles": [str(path) for path in extern_paths],
        "runtimeApiFiles": [{"kind": kind, "path": str(path)} for kind, path in json_paths],
        "counts": {
            "externSignatures": len(signatures),
            "runtimeApiLookupKeys": len(runtime_api_keys),
            "missingFromRuntimeApi": len(missing_from_runtime_api),
            "extraRuntimeApi": len(extra_runtime_api),
            "signatureChanged": 0,
            "renamed": 0,
        },
        "missingFromRuntimeApi": missing_from_runtime_api,
        "extraRuntimeApi": extra_runtime_api,
        "signatureChanged": [],
        "renamed": [],
    }
    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    return missing_from_runtime_api


def main() -> int:
    args = parse_args()
    repo_root = (args.repo_root or default_repo_root()).resolve()
    runtime_api_dir = resolve_runtime_api_dir(args, repo_root)

    try:
        extern_paths = [p.resolve() for p in collect_extern_paths(args, repo_root)]
    except FileNotFoundError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    if runtime_api_dir is not None:
        if not args.icalls:
            args.icalls.append(runtime_api_dir / "icalls.json")
        if not args.intrinsics:
            args.intrinsics.append(runtime_api_dir / "intrinsics.json")
        if not args.pinvokes:
            args.pinvokes.append(runtime_api_dir / "pinvokes.json")

    if not extern_paths:
        print("error: specify at least one --externs file, --externs-dir, or --profile", file=sys.stderr)
        return 2

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
    for path in args.pinvokes:
        json_paths.append(("pinvokes", path.resolve()))

    if not json_paths:
        print("error: specify at least one --icalls or --intrinsics file", file=sys.stderr)
        return 2

    all_missing: list[dict] = []
    for kind, path in json_paths:
        if not path.is_file():
            print(f"error: {kind} file not found: {path}", file=sys.stderr)
            return 2
        entries = load_runtime_api_entries(path, kind)
        missing = check_json_file(path, signatures, kind, args.profile)
        whitelisted_count = sum(1 for e in entries if isinstance(e.get("name"), str) and is_whitelisted(kind, e["name"]))
        print(
            f"{path.name}: {len(entries)} entries, {len(missing)} not in externs"
            + (f" ({whitelisted_count} whitelisted)" if whitelisted_count else "")
        )
        all_missing.extend(missing)

    missing_from_runtime_api: list[str] = []
    if args.diff_report:
        diff_report_json_paths = collect_diff_report_runtime_api_paths(json_paths, runtime_api_dir)
        missing_from_runtime_api = write_diff_report(
            args.diff_report.resolve(), extern_paths, diff_report_json_paths, signatures, all_missing
        )
        print(
            f"Wrote diff report to {args.diff_report}: "
            f"{len(missing_from_runtime_api)} missing_from_runtime_api, {len(all_missing)} extra_runtime_api"
        )

    if args.fail_on_missing_externs and missing_from_runtime_api:
        print("\nExtern signatures missing from runtime API JSON:")
        for signature in missing_from_runtime_api:
            print(f"  {signature}")
        return 1

    if all_missing:
        print("\nRuntime API contract issues:")
        for item in all_missing:
            func = item.get("func")
            func_suffix = f"  func={func}" if func else ""
            reason = item.get("reason")
            reason_suffix = f"  reason={reason}" if reason else ""
            print(f"  [{item['file']}] {item['name']}{func_suffix}{reason_suffix}")
        return 1

    print("All entries matched extern signatures.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
