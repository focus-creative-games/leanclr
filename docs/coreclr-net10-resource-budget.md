# Minimal net10 resource budget

Local targets: Windows x64 / MSVC Release and Node Wasm32, CoreLib 10.0.8 of matching pointer width. Five fresh processes per cell, 100,000 lookups per process, 8,192 real managed arrays. Samples are interleaved baseline/current. Nanosecond microbenchmarks depend on CPU, allocator and host scheduling.

`prepare-resource-baseline.py` creates an isolated snapshot of current sources under out/, replacing only gchandle.cpp with its linear-slot implementation from ad0c8624. Both probes contain the same warmup, facade-cap checks, heap workload and metrics. It never checks out, resets or edits a branch. This controlled comparison supersedes the initial saved executable, whose workload predated the extra facade warmup. This is not an IL2CPP benchmark.

| Handles | Native linear, ns | Native indexed, ns | Wasm linear, ns | Wasm indexed, ns |
| --- | ---: | ---: | ---: | ---: |
| 1024 | 602.012 | 1.812 | 474.159 | 5.799 |
| 4096 | 2212.642 | 3.721 | 1873.875 | 7.94 |
| 16384 | 9640.677 | 5.843 | 7451.063 | 9.525 |

Slot lookup uses exact active slot addresses without dereferencing untrusted input, allocates fallibly before publication, removes freed entries and preserves slot identity through rehash. Expected lookup is O(1), with O(n) storage and O(n) worst-case hash collisions. At 16,384 live handles the extra 32,768-entry address table retains 256 KiB on x64 / 128 KiB on Wasm32, alongside the existing ID index. Intermediate resize storage also affects peak memory.

Interior membership uses the GC stage's sorted arena range index: O(log arenas), then constant-time small-block membership. Cold registration/removal shifts the range vector, O(arenas). There is no assembly/method scan in either hot lookup. At 16,384 handles the valid-interior/invalid-pointer pair median is 33.481 ns native / 54.971 ns Wasm; this change targets slot lookup, not a new membership algorithm.

| Metric, 16,384 handles | Native baseline | Native indexed | Wasm baseline | Wasm indexed |
| --- | ---: | ---: | ---: | ---: |
| Managed live heap, bytes | 2416280 | 2416280 | 2283464 | 2283464 |
| Peak process working set, bytes | 28606464 | 29061120 | unavailable | unavailable |
| Peak Wasm linear memory, bytes | N/A | N/A | 24903680 | 24772608 |
| Executable / Wasm module bytes | 1140224 | 1140224 | 998082 | 997779 |

Wasm JS loader: 71327 bytes for both builds. The Wasm module shrinks by 303 bytes in this probe. Managed heap is unchanged: the index is native storage. Linear memory is monotonic, so its final size is its peak here. Its growth/allocator fragmentation makes this particular indexed run reach fewer pages despite extra retained index storage; this is not a claim that indexing always saves memory. Native working-set growth and the explicit backing-array cost are the resource tradeoff for avoiding linear scans. These sizes are uncompressed resource probes, not an application download-size estimate.

RuntimeType facade registration defaults to 16,384 canonical identities, configurable on the runtime owner thread via Settings or engine startup. At the cap, new identities return OutOfMemory; repeated failures do not grow the registry and existing identities remain valid. Facade/auxiliary records share a correctly aligned allocation (including Wasm32). Layout-derived flags are cached per bounded facade and refreshed when class layout flags change; no separate unbounded layout cache is introduced. Metadata remains process-lifetime storage; this is not a total VM memory quota or unloading mechanism.

The untargeted Wasm membership median is 54.971 ns indexed versus 52.237 ns baseline (+5.2%). Samples span 49.795–60.586 versus 49.492–60.707 ns, respectively; the ranges overlap almost completely. The algorithm is unchanged, while allocation/layout and host scheduling vary. No membership speedup or statistically established regression is claimed from this five-run probe; this fluctuation is reported rather than hidden behind the slot gain.

## Reproduction

Use a full Git checkout containing ad0c8624; fetch history first if the checkout is shallow. Prepare the source snapshot, then configure/build both the normal src/tests/net10-contracts and snapshot out/net10-linear-baseline/src/tests/net10-contracts with identical compiler, Release and host options. Native Windows builds use -A x64. Wasm builds use the Emscripten toolchain, -G Ninja, -DLEANCLR_HOST=PORTABLE and -DCMAKE_CROSSCOMPILING_EMULATOR=node.

```powershell
python scripts/dotnet10/prepare-resource-baseline.py
python scripts/dotnet10/fetch-corelib.py
python scripts/dotnet10/resource-budget.py --runner out/net10-contracts/Release/net10-resources.exe --baseline out/net10-linear-baseline/native/Release/net10-resources.exe --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8" --output out/net10-resource-controlled-native.json
python scripts/dotnet10/resource-budget.py --runner out/net10-contracts-wasm/net10-resources.js --baseline out/net10-linear-baseline/wasm/net10-resources.js --node --runtime out/net10-runtime-win-x86/runtimes/win-x86/lib/net10.0 --output out/net10-resource-wasm-comparison.json
```

Raw samples, medians and binary sizes are retained in those JSON outputs. `net10-platforms.yml` uploads the Wasm comparison when run remotely. Windows native and Wasm handle/resource contracts pass 3/3; Linux/ARM64 and UBSan remain unexecuted locally because WSL cannot start. CI definitions are not claimed as platform passes.
