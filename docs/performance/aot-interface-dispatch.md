# AOT interface dispatch within loops

This round adds the opt-in generator flag `--leanaot-interface-call-cache`.
It reuses a resolved interface target within one AOT method invocation. The
default remains off while the stack/code-size tradeoff is reviewed. Neither
runtime interface resolution semantics nor AOT return signatures change.

## Scope and lifetime

Each selected callsite gets one zero-initialized stack-local entry containing
receiver class, declaration method and implementation method. A hit requires
both class and declaration identity. The receiver itself is passed anew to the
existing call bridge; the cache never retains a managed object. Distinct
instances of the same class can therefore share the resolved implementation
while retaining their own state.

On a miss, the existing virtual-method resolver handles interface variance,
generic inflation and AOT/interpreter target selection. Only successful
resolutions replace the entry. Invocation continues through the existing
bridge, preserving the actual implementation's generic context and return
protocol. Exceptions and null checks retain their existing IL positions and
normal-path EH/interpreter handoff. No object or argument values are cached.

Stack ownership makes recursive and reentrant calls independent, requires no
locking, and cannot survive runtime shutdown. Entries contain metadata pointers
only, add no GC roots or heap index, and assume the same immutable resolved
class/vtable lifetime as existing runtime dispatch. This does not add support
for hot replacement of vtables.

The generator selects interface `callvirt` instructions inside a backward-branch
interval, up to four per method. This is a conservative loop heuristic, not PGO
or a guarantee of repeated execution. Straight-line interface calls, ordinary
class virtual calls, `ldvirtftn`, devirtualized calls and extra sites beyond the
budget keep the previous path. Declared cache storage is three pointers per
site: 24 bytes on x64 or 12 bytes on x86/Wasm, at most 96/48 bytes per invocation.
Actual frame sizes also depend on register allocation, spills and optimization.
The first iteration misses, and alternating receiver classes miss repeatedly;
there is no global or cross-invocation reuse.

## Validation and reproduction

Eleven formal `managed/AotTests/TC_InterfaceDispatch.cs` tests cover monomorphic
and polymorphic receivers, same-class distinct objects, interpreter targets,
void/reference bridges, generic contexts, variance, boxed value receivers,
exceptions and null after warmup, recursive frames and the four-site budget.
The native host verifies the actual loop and target backends, successful error
handoff, three-pointer trivial layout, declaration-key invalidation, missing
interface errors and preservation of the previous entry on failure. Generator
checks enforce opt-in behavior, no straight-line cache and the site budget.
The existing 24 formal generic cases run alongside these tests.

```powershell
# Cached mode: repeat with --lightweight-returns for the other return protocol.
python scripts/test/global-generics.py --interface-call-cache --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/global-generics.py --interface-call-cache --config Debug
python scripts/test/global-generics.py --interface-call-cache --arch Win32
# Cache-disabled control.
python scripts/test/global-generics.py --output out/interface-dispatch/baseline --target all --emsdk D:/workspace/wasmclr/emsdk

# Build first; sample only after other compilation/test processes finish.
python scripts/benchmarks/compare_interface_dispatch.py --build-only --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_interface_dispatch.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

The comparison uses byte-identical managed input and otherwise identical
generator/compiler options. Four workloads cover a monomorphic interface loop,
alternating classes, a generic interface method and a straight-line helper
control. Both entries and the concrete generic override are verified as AOT.
Metadata is warm, but each timed AOT invocation starts with empty local caches.
Seven process samples are interleaved after compilation; checksums must agree.
There is no new IL2CPP, browser, cold-start or game-wide measurement. Previous
performance snapshots remain unchanged.

## Results and decision

All 24 final configurations passed: 16 cached configurations (Result/lightweight
returns, generic discovery on/off, x64 Release/Debug, Win32 Release and Wasm
Release) and eight uncached controls (both return protocols and discovery modes
on x64 Release and Wasm). Each executes 35 formal methods, totaling 840 method
executions, including 264 new interface case executions. Runtime regressions
passed 41/41. Local runs used Windows/Node; remote CI, Linux/ARM, browser and
full managed/GC regression suites were not rerun in this round.

The measured pair uses the default Result protocol, MSVC 19.44.35229.0 and
Emscripten/Clang 22.0.0 with Node v22.12.0. Each workload runs 5,000,000 iterations
per sample. CPU frequency and background activity were not controlled. Values
below are seven-sample medians in ns/iteration; ranges are in the raw data.

| Workload | x64 original | x64 cache | Wasm original | Wasm cache |
| --- | ---: | ---: | ---: | ---: |
| Single receiver class | 5.138 | 2.516 | 7.910 | 3.549 |
| Alternating receiver classes | 5.719 | 6.504 | 7.248 | 8.082 |
| Generic interface method | 17.260 | 2.460 | 24.510 | 3.714 |
| Straight-line helper control | 7.706 | 8.020 | 8.894 | 8.817 |

Single-class loop time falls about 51% on x64 and 55% on Wasm. The generic loop
falls about 86%/85%, since hits avoid both interface resolution and repeated
generic-method lookup. These two workloads have nonoverlapping baseline/cache
sample ranges. Alternating-class medians instead rise about 14%/12%; their
ranges overlap, but every iteration still misses and pays the additional cache
checks/stores. The unchanged straight-line helper has overlapping sample
ranges and provides no evidence of an improvement. These measurements do not
support enabling caching indiscriminately.

The test executable is 882,176 bytes in both x64 builds. Wasm grows from 833,546
to 833,736 bytes (+190), its code section from 668,258 to 668,451 (+193), and gzip
from 266,709 to 266,834 (+125). This fixture has three cached callsites across
three methods; these are whole-test-artifact differences and exclude external
BCL, IL and resources. No runtime-wide memory or code-size reduction is claimed.

Keep the option **off by default**. It is useful for repeated calls at sites
with stable receiver classes, especially generic interfaces. A later reviewed
step can use measured receiver profiles or a bounded polymorphic policy to
address changing receiver classes; this round does not add that machinery.
The option does not change internal registration-table layout or public APIs.

Raw samples, checksums, source/generated-file hashes, byte-identical managed
input hashes, toolchain/build commands, artifact sizes and validation log
hashes are recorded in
[aot-interface-dispatch.json](data/aot-interface-dispatch.json).
