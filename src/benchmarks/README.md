# Performance experiments

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
