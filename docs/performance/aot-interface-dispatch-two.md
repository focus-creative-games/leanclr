# Two-entry AOT interface cache

This round follows the alternating-receiver slowdown found in the
[single-entry experiment](aot-interface-dispatch.md). It adds an optional
two-entry policy while preserving single-entry behavior and keeping all
interface caching off by default.

```text
--leanaot-interface-call-cache --leanaot-interface-cache-entries 2
```

Capacity defaults to 1. Values other than 1 or 2 are rejected before generation.
Setting capacity without the enable flag does not allocate caches. The generic
report records both enable state and effective capacity (zero when disabled).

## Resolution, replacement and lifetime

The two-entry cache stores a shared declaration method, two receiver classes,
two implementation methods and a one-byte replacement cursor. Both class and
declaration identity must match. Hits return the implementation without
changing the cursor or moving entries. A miss uses the existing resolver, then
replaces a slot in round-robin order. This lets a new pair of receiver classes
displace an old pair; the first receiver is not permanently pinned.

Failed resolution leaves both entries and the cursor untouched. A successful
declaration change clears the old second entry and starts replacement at slot
zero. Existing invocation bridges still select the implementation's generic
context and return protocol, including interpreter targets. Cache ownership,
null checks, exception IL positions and interpreter handoff are unchanged.
All stored pointers refer to metadata, with no heap index or managed GC roots.

The measured layout, including cursor padding, is 48 bytes on x64 and 24 bytes
on x86/Wasm. The generator permits **two sites per method** in two-entry mode,
compared with four 24/12-byte sites in single-entry mode. Thus both modes cap
declared cache storage at **96 bytes on x64 / 48 bytes on x86/Wasm per invocation**.
Actual frames depend on register allocation and spills; this is not a measured
total stack-frame size. Both policies still select backward-branch loop
candidates and leave straight-line and excess sites uncached.

Two entries cannot cover an arbitrary polymorphic site. Three alternating
classes can evict each other on every iteration. Reducing the site budget also
means methods with several distinct interface callsites may lose coverage.
These tradeoffs are part of the benchmark, not assumed to be harmless.

## Tests and reproduction

Three new formal managed cases cover three receiver classes, a change from
one receiver pair to another, and an exception from the warmed second target.
They run with the previous eleven interface cases and 24 generic cases.
Native checks cover layout and trivial lifetime, both target identities,
hit-without-rotation, round-robin replacement, phase adaptation, declaration
changes and error preservation. Generator checks enforce the mode-specific
site limit and the common twelve-pointer-word storage budget. CLI checks
exercise invalid capacities and capacity without enabling caching.

```powershell
# Repeat with --lightweight-returns for the second return protocol.
python scripts/test/global-generics.py --interface-call-cache --interface-cache-entries 2 --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/global-generics.py --interface-call-cache --interface-cache-entries 2 --config Debug
python scripts/test/global-generics.py --interface-call-cache --interface-cache-entries 2 --arch Win32
# Single-entry and disabled controls.
python scripts/test/global-generics.py --interface-call-cache --output out/interface-dispatch/cached --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/global-generics.py --interface-cache-entries 2 --output out/interface-dispatch/baseline --target all --emsdk D:/workspace/wasmclr/emsdk

# Build all three modes first. Wait for other builds/tests before sampling.
python scripts/benchmarks/compare_interface_dispatch.py --two-entry --build-only --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_interface_dispatch.py --two-entry --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

The comparison uses byte-identical managed inputs and otherwise matching
compiler/generator settings. Seven workloads include the original four, plus
three alternating types, receiver-pair phase changes, and five callsites in
one loop. The five-site workload reports time per iteration, containing five
interface invocations; the other workloads perform one interface invocation
per iteration. Metadata is warm but each invocation starts with empty local
caches. Native entry and generic implementation backends are verified as AOT.
No new IL2CPP, browser, cold-start or game-wide measurement is implied.

## Validation results

All 32 configurations passed: sixteen two-entry configurations across both
return protocols, generic discovery on/off, x64 Release/Debug, Win32 Release
and Wasm Release; eight single-entry controls and eight disabled controls on
x64/Wasm Release. Each executes 38 formal methods (14 interface and 24 generic),
totaling 1,216 executions. Runtime regressions passed 41/41. Local execution
was Windows/Node; remote CI, Linux/ARM, browser and the full managed/GC suites
were not rerun this round. CI now includes all three cache modes.

## Performance and size

The three measured builds use the default Result return protocol, identical
managed DLL bytes, MSVC 19.44.35229.0, Emscripten/Clang 22.0.0 and Node v22.12.0.
Seven process samples, 5,000,000 iterations per workload, are interleaved only
after all compilation/tests finish. No samples were discarded. Frequencies
and background activity were not controlled. Values are ns/iteration medians.

| Workload | x64 off | x64 one | x64 two | Wasm off | Wasm one | Wasm two |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| One class | 5.164 | 2.774 | 2.445 | 7.196 | 3.647 | 3.847 |
| Two alternating classes | 5.400 | 6.660 | 3.402 | 7.231 | 8.109 | 4.106 |
| Generic interface | 17.016 | 2.803 | 3.147 | 22.326 | 3.624 | 3.696 |
| Three alternating classes | 5.490 | 6.219 | 6.750 | 7.605 | 7.890 | 8.424 |
| Receiver-pair phase change | 5.058 | 5.837 | 3.450 | 7.697 | 7.975 | 4.299 |
| Five sites per iteration | 32.518 | 18.717 | 24.957 | 35.724 | 21.351 | 30.173 |
| Straight-line control | 6.896 | 8.610 | 8.019 | 8.782 | 8.881 | 8.798 |

Wasm two-class time falls about **43% versus disabled** and **49% versus one
entry**, with nonoverlapping sample ranges. The receiver-pair phase workload
also benefits. Single-class and generic-interface Wasm medians change by about
+5.5%/+2.0% versus one entry, with overlapping ranges; a second entry is not a
universal improvement even before accounting for the doubled per-site storage.
Three-class Wasm time rises about 11% versus disabled (ranges overlap), matching
the policy's capacity limit. Five-site time is about 41% above single-entry
mode, with nonoverlapping ranges, because only two sites retain caches.

The x64 straight-line control has noticeably different medians even though
both generated control sections are byte-identical across modes; its sample
ranges overlap. Toolchain/layout/system contributions were not isolated, so
exact native percentage attribution remains exploratory. The unchanged Wasm
control is much closer across modes. All raw samples and controls are retained.

| Artifact bytes | Off | One entry | Two entries |
| --- | ---: | ---: | ---: |
| x64 executable | 885248 | 886272 | 886784 |
| Wasm file | 836429 | 837012 | 837675 |
| Wasm code section | 670687 | 671270 | 671933 |
| Wasm gzip | 267383 | 267614 | 267795 |

These are whole-test-artifact sizes, excluding external BCL/IL/resources.
Two-entry mode adds 1,246 Wasm bytes versus disabled, or 663 versus one entry.
It does not reduce code size. The generated fixture contains nine single-entry
sites or seven two-entry sites, with the same maximum twelve-word declared
cache budget per method. This is not a runtime-wide stack or memory measurement.

Keep capacity 1 as the enabled-mode default and keep caching itself **off by
default**. Capacity 2 is an explicit option for builds with stable one/two-class
loop sites; higher polymorphism and multiple callsites still need measurement.
No registration ABI, public API, unbounded cache or adaptive profiling policy
is introduced. The previous single-entry measurements remain frozen in their
original report and JSON.

Raw rows, checksums, source/generated/control hashes, toolchains, build
commands, artifact sizes and validation evidence are in
[aot-interface-dispatch-two.json](data/aot-interface-dispatch-two.json).
