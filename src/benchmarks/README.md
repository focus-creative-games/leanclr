# Performance experiments

## Win64 IL instruction families

`il-families` compares 26 same-IL kernels on the pure LeanCLR interpreter,
default LeanAOT, Unity 2022.3.62f3 IL2CPP and .NET 10.0.8 optimized JIT.
It covers integer/floating arithmetic, conversion, checked operations, branches,
arrays, fields, byref, calls, virtual/interface/delegate dispatch, structs,
generics, type checks, boxing, allocations and exception paths.

```powershell
python scripts/benchmarks/run_il_families.py --unity D:/UnityHubs/2022.3.62f3
python scripts/benchmarks/report_il_families.py
```

The runner validates results, pins measurement processes to one logical CPU,
calibrates per-engine iteration counts and records seven randomized rounds.
See [the measured report](../../docs/performance/il-families-win64.md) for build
settings, raw samples, variability, IL inventories and AOT fallback details.
The .NET configuration disables tiering and ReadyToRun; IL2CPP is a standalone
CLI build, not a Unity Player. These are kernel timings, not per-opcode costs.

For generated-code ablations of Delegate, Cast and NewArray, run
`python scripts/benchmarks/probe_il_hotspots.py`, then use `--sweep-only` for
NewArray count scaling. See [the hotspot analysis](../../docs/performance/il-hotspots-win64.md).
These probes reuse the baseline runtime library and do not modify production
code. GC counters show the LeanCLR allocation loop does not automatically
collect; its allocation results measure heap growth, not GC steady state.

Production optimization verification uses the preserved baseline binaries:

```powershell
python scripts/test/aot-hotspots.py --all-protocols
python scripts/test/aot-hotspots.py --gc-debug
python scripts/benchmarks/run_il_optimized.py
python scripts/benchmarks/report_il_optimized.py
```

This builds production generated code in a separate output directory, measures
all 26 kernels with a same-session old AOT control, then repeats the three
hotspots with identical iteration counts on four native backends. See the
[production report](../../docs/performance/il-hotspots-optimized-win64.md).

For the subsequent allocator optimization, preserve the c64a033d binaries in
`out/performance/il-families-optimized/` and use a separate output directory:

```powershell
python scripts/benchmarks/run_il_optimized.py --baseline out/performance/il-families-optimized --output out/performance/il-array-allocator
python scripts/benchmarks/profile_array_allocator.py
python scripts/benchmarks/report_array_allocator.py
```

The [allocator report](../../docs/performance/newarray-allocator-win64.md) includes
count scaling, separate Win64 memory probes, metadata capacity costs, and
virgin/recycled-block, OOM, quarantine and finalizer validation. Memory polling
runs are excluded from latency summaries.

These are isolated P0/P1 experiments. They do not change the production Result
ABI or LeanAOT code generator. See the measured [review report](../../docs/performance/p0-p1.md).

## Return protocols

`result-abi` compares current Result, trivial wrappers, packed scalars, status +
output, per-invocation context, C++/Wasm EH and a successful-only direct baseline.
Five payload types are measured through non-tail call chains. CTest checks mixed
and all-error cases, full-width uint32 boundary values, successful null pointers,
and cross-protocol deterministic output. The trivial wrapper only supports
trivial payloads; it is not a general replacement for Result<T>.

```powershell
python scripts/benchmarks/run_protocols.py --target native
python scripts/benchmarks/run_protocols.py --target wasm --emsdk D:/path/to/emsdk
```

To run semantics only, without the measurement driver:

```powershell
cmake -S src/benchmarks/result-abi -B out/protocol-check -A x64 -DCMAKE_TRY_COMPILE_CONFIGURATION=Release
cmake --build out/protocol-check --config Release
ctest --test-dir out/protocol-check -C Release --output-on-failure
```

Use `--opt s`, or `--opt 3 --lto`, for additional Wasm configurations. The runner
records all commands, raw samples, min/median/max, checksums, code/raw/gzip sizes
and Wasm LLVM ABI declarations under `out/performance`. Do not run competing
builds or benchmark processes while measuring. Do not use process wall time as
startup latency. CI checks correctness, not noisy timing thresholds.

## Same-source AOT baseline

`aot-baseline/Benchmarks.cs` exercises arithmetic, small calls, properties, arrays,
interfaces, structs, generics and exceptions. The native host validates AOT entry
selection. Generic helper fallback and EH method fallback are explicitly labeled;
the generator currently does not AOT those methods in this benchmark.

```powershell
python scripts/benchmarks/run_aot.py --unity D:/UnityHubs/2022.3.62f3 --emsdk D:/path/to/emsdk
python scripts/benchmarks/summarize.py
```

The full AOT build driver currently requires Windows and Framework CSC. Omit
`--unity` to measure LeanAOT and C++ only; omit `--emsdk` for native only.
`--skip-build` remeasures existing artifacts. The report publisher expects the
complete native/Wasm datasets, including the IL2CPP native comparison.

IL2CPP uses its installed Unity BCL; LeanAOT uses the repository Mono BCL.
NoInlining behavior differs. If the Editor omits the standalone CLI thread-hook
helper, the runner builds `il2cpp-thread-hook.cpp`, preserving the runtime's
thread-detach callback contract, and replays its link steps using installed
baselib. It never modifies the Editor or generated method implementations.

The Node-only host mount is test infrastructure, not a browser integration.
IL2CPP/WebGL timing, allocation pressure, application workloads and isolated cold
startup latency are not measured here. Neither the binary size tables nor the
minimal EH experiment establish a production deployment size or a CLR-complete
exception implementation.
