# Profile-Guided AOT: `pgo2aot`

This guide describes the **Profile Guided Optimization (PGO)** workflow for LeanCLR: collect runtime call profiles, convert them to a **PGO rule XML** file with `pgo2aot`, and feed that file to **LeanAOT**.

**Related documents:**

| Document | Contents |
|----------|----------|
| [aot.md](aot.md) | LeanAOT workflow, build, runtime registration |
| [aot-rule-file.md](aot-rule-file.md) | Hand-written `aot.xml` (include/exclude defaults, wildcards) |
| [unity.md](unity.md) | Unity WebGL integration and `LEANAOT_EXTRA_ARGS` |

---

## 1. End-to-end workflow

```text
┌─────────────────────┐     ┌──────────────────┐     ┌─────────────────┐     ┌──────────┐
│ LeanAOT build with  │     │ Run app / game   │     │ pgo2aot         │     │ LeanAOT  │
│ --leanaot-enable-   │ ──► │ (profiled build) │ ──► │ profile JSON    │ ──► │ rebuild  │
│ pgo-profile         │     │ export JSON      │     │ → pgo-aot.xml   │     │ with PGO │
└─────────────────────┘     └──────────────────┘     └─────────────────┘     └──────────┘
```

1. **Instrumented build** — Generate AOT code with `--leanaot-enable-pgo-profile` so the runtime records per-method `calls` and weighted `cost`.
2. **Profile collection** — Run the application under realistic load. Export profile data as JSON (see Section 2).
3. **Rule generation** — Run `pgo2aot` to select hot methods and emit `pgo-aot.xml`.
4. **Targeted AOT rebuild** — Pass the PGO rule file to LeanAOT with `--leanaot-pgo-rule-file` (repeatable). Combine with a broad `aot.xml` exclude policy when you want “AOT only what the profile says” (Section 6).

Typical pairing:

- `aot.xml` — `aot="0"` on large assemblies (exclude almost everything by default).
- `pgo-aot.xml` — exact hot-method list from profiling (force-include those methods).

PGO rules are **additive only**: they can turn **off → on** for a method, never **on → off**. See [Section 6](#6-pgo-rule-xml-vs-hand-written-aotxml).

---

## 2. Profile JSON format

`pgo2aot` reads one or more JSON files. Each file is a **JSON array** of objects:

```json
[
  {
    "assembly": "mscorlib",
    "signature": "System.Boolean System.Char::IsWhiteSpace(System.Char)",
    "calls": 12034,
    "cost": 482910
  }
]
```

| Field | Type | Description |
|-------|------|-------------|
| `assembly` | string | Assembly **short name** (no `.dll`), e.g. `mscorlib`, `Assembly-CSharp` |
| `signature` | string | Full managed method signature (same string family as LeanAOT / dnlib `MethodDef.FullName`) |
| `calls` | number | Call count observed during profiling |
| `cost` | number | Weighted cost (implementation-defined; higher = hotter) |

### Exporting from the runtime

Managed API (`LeanCLR.Profile`):

- `GetGlobalStatsJson()` / `ExportGlobalStatsJson(path)` — cumulative stats since startup
- `GetPeriodStatsJson()` / `ExportPeriodStatsJson(path)` — stats for the current profiling period

Reset helpers: `ResetGlobalStats()`, `ResetPeriodStats()`, `EndPeriodStats()`.

Profiling must be enabled in the instrumented build (`--leanaot-enable-pgo-profile`).

### Multiple input files

Pass `--input` more than once. `pgo2aot` merges records by `(assembly, signature)` and keeps the **maximum** `calls` and `cost` for each key.

### Sample data

Example profile and generated outputs live under:

`src/tools/pgo2aot/samples/`

| File | Description |
|------|-------------|
| `global.json` | Example profile (~1580 records from a Unity session) |
| `aot-threshold.xml` | Threshold strategy output |
| `aot-top-n.xml` | Top-N strategy output |
| `aot-top-percent.xml` | Top-percent strategy output |
| `aot-pareto.xml` | Pareto strategy output |

> **Note:** Keep profile JSON **outside** the `pgo2aot` project directory root if the file is named `global.json`. The .NET SDK treats `global.json` next to a `.csproj` as an SDK pin file and `dotnet build` will fail. The repository sample uses `samples/global.json` for this reason.

---

## 3. Building `pgo2aot`

From repository root or `src/tools/pgo2aot/`:

```bat
src\tools\pgo2aot\build.bat
```

```bash
src/tools/pgo2aot/build.sh
```

Optional configuration: `Debug` or `Release` (default `Release`). Equivalent:

```bat
dotnet build src\tools\pgo2aot\Pgo2Aot.csproj -c Release
```

Published binary:

`out/dotnet/Pgo2Aot/Release/net8.0/pgo2aot.dll`

Run with `dotnet pgo2aot.dll ...` or invoke the `.dll` directly.

Requires **.NET 8** (or newer with `RollForward`).

---

## 4. Command-line reference

```text
pgo2aot --input <profile.json> [--input <more.json> ...]
        --output <pgo-aot.xml>
        --strategy threshold | top | pareto
        [strategy-specific options]
        [--sort-by cost | calls]
```

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `-i`, `--input` | yes | — | Profile JSON path; repeat to merge |
| `-o`, `--output` | yes | — | Output PGO rule XML path |
| `--strategy` | yes | — | Selection strategy (Section 5) |
| `--min-calls` | threshold | `1` | Minimum call count |
| `--min-cost` | threshold | `0` | Minimum weighted cost |
| `--top-n` | top | — | Keep first N methods after sorting |
| `--top-percent` | top | — | Keep first N% of methods (e.g. `10` = top 10%) |
| `--pareto-ratio` | pareto | `0.8` | Cumulative metric fraction to keep (e.g. `0.8` = 80%) |
| `--sort-by` | top, pareto | `cost` | Ranking metric: `cost` or `calls` |

`top` requires **either** `--top-n` **or** `--top-percent` (not both required, but one must be set).

On success, the tool prints:

```text
Selected 90 method(s) from 1580 profile record(s) -> path\to\pgo-aot.xml
```

---

## 5. Selection strategies

All strategies sort candidates by `--sort-by` (`cost` by default), then break ties by the other metric, then by assembly and signature.

### 5.1 `threshold`

Keep every record where **both** conditions hold:

- `calls >= --min-calls`
- `cost >= --min-cost`

Use when you want a simple “hot enough” filter without a fixed budget.

```bat
dotnet pgo2aot.dll ^
  --input samples\global.json ^
  --output pgo-threshold.xml ^
  --strategy threshold ^
  --min-calls 100 ^
  --min-cost 10000
```

### 5.2 `top`

Keep the first **N** methods or the first **N%** after sorting.

```bat
REM Top 50 by cost
dotnet pgo2aot.dll --input samples\global.json --output pgo-top50.xml ^
  --strategy top --top-n 50 --sort-by cost

REM Top 10% by cost
dotnet pgo2aot.dll --input samples\global.json --output pgo-top10pct.xml ^
  --strategy top --top-percent 10 --sort-by cost
```

Use when you have a fixed AOT code-size or compile-time budget.

### 5.3 `pareto`

Walk the sorted list and accumulate `calls` or `cost` until the running total reaches `--pareto-ratio` × (total metric). This implements an 80/20-style cutoff (default ratio `0.8`).

```bat
dotnet pgo2aot.dll --input samples\global.json --output pgo-pareto.xml ^
  --strategy pareto --pareto-ratio 0.8 --sort-by cost
```

Use when you want “cover most of observed runtime cost with relatively few methods”.

---

## 6. PGO rule XML vs hand-written `aot.xml`

PGO output uses a **different, machine-oriented schema** from [aot-rule-file.md](aot-rule-file.md):

```xml
<?xml version="1.0" encoding="utf-8"?>
<aot>
  <assembly name="mscorlib">
    <method signature="System.Boolean System.Char::IsWhiteSpace(System.Char)" />
  </assembly>
  <assembly name="Assembly-CSharp">
    <method signature="System.Void MyGame.App::Update()" />
  </assembly>
</aot>
```

| | Hand-written `aot.xml` | PGO `pgo-aot.xml` |
|--|------------------------|-------------------|
| Assembly attribute | `fullname`, optional `aot` default | `name` (short name only) |
| Method | `name`, optional `signature`, required `aot="0\|1"` | `signature` only (implicit include) |
| Wildcards | Supported on type/method patterns | **Not supported** — exact signatures only |
| Semantics | Include/exclude policy | **Additive include** only |

**Additive semantics:** If a method is listed in a PGO file, LeanAOT **includes it in AOT** even when `aot.xml` would exclude it. PGO never removes a method that `aot.xml` or default policy already includes.

Repository example (tests): `src/tests/aot-tester/pgo-aot.xml`.

---

## 7. Consuming PGO rules in LeanAOT

Pass one or more PGO rule files:

```text
LeanAOT ... --leanaot-pgo-rule-file path\to\pgo-aot.xml
```

- Repeat the option to merge multiple files (union of listed methods).
- Paths may be absolute or relative to the current working directory.

**Unity:** add to `LEANAOT_EXTRA_ARGS` before building WebGL (see [unity.md](unity.md)):

```text
--leanaot-pgo-rule-file=C:\path\to\pgo-aot.xml
```

**Profile instrumentation** (first profiling build):

```text
--leanaot-enable-pgo-profile
```

---

## 8. Recommended production loop

1. **Baseline policy** — Author `aot.xml` with `aot="0"` on game and engine assemblies you want to keep small in native code.
2. **Profile build** — WebGL/player build with `--leanaot-enable-pgo-profile`.
3. **Representative session** — Play through loading, combat, UI, etc.; export `global.json` (or merge several session files).
4. **Generate rules** — Run `pgo2aot` with `pareto` or `top` strategy; inspect method count and spot-check signatures.
5. **Release build** — Same `aot.xml` plus `--leanaot-pgo-rule-file=...` **without** profile instrumentation.
6. **Iterate** — Re-export after major gameplay or code changes.

---

## 9. FAQ

**Must `signature` match my C# source?**  
It must match LeanAOT’s internal method signature string (dnlib `MethodDef.FullName` style), including generic arity markers and nested type names. If LeanAOT cannot match an entry, check `MethodMap.tsv` from a prior build or enable verbose logging.

**Why do some signatures contain `*`?**  
Pointer types appear as `System.UInt32*`, `System.Byte*`, etc. That `*` is part of the type name, not a glob wildcard.

**Can I use `pgo-aot.xml` instead of `aot.xml`?**  
No. They serve different roles. PGO files only **add** AOT candidates; they do not provide assembly-wide exclude defaults. Use both when you want selective AOT.

**What if I pass no PGO rule file?**  
LeanAOT behaves as documented in [aot-rule-file.md](aot-rule-file.md); PGO has no effect.

---

## 10. Tool source layout

| Path | Role |
|------|------|
| `src/tools/pgo2aot/Program.cs` | CLI entry |
| `src/tools/pgo2aot/ProfileJsonReader.cs` | JSON load and merge |
| `src/tools/pgo2aot/ProfileSelection.cs` | `threshold` / `top` / `pareto` |
| `src/tools/pgo2aot/AotXmlWriter.cs` | PGO XML output |
| `src/leanaot/LeanAOT.GenerationPlan/PgoMethodIncludeIndex.cs` | LeanAOT consumer |
| `src/libraries/LeanCLR/Profile.cs` | Managed export API |
| `src/runtime/profile/` | Native profiler |
