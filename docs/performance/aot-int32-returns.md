# AOT int/uint packed return trial

`--leanaot-packed-int32-returns` opts generated AOT `int`/`uint` methods into a
64-bit result representation. It is independent of
`--leanaot-lightweight-returns`; both defaults remain off. Ordinary runtime
APIs, interpreter invokers and public C APIs retain their existing contracts.

## Representation and eligibility

`AotInt32Result<int32_t/uint32_t>` stores the entire payload in the low 32 bits.
A zero high word means success; a nonzero high word holds `RtErr`, leaving the
low word zero on failure. No integer payload is reserved as an error sentinel.
Unsigned conversion encodes negative values modulo 2^32; a fixed-size memcpy
decodes their signed bit representation without an out-of-range unsigned-to-
signed cast. The wrapper is eight bytes and trivially copyable in Release and
Debug. It has no diagnostic destructor, shared error context or heap storage.

Only IL `I4`/`U4` return signatures qualify. Enums, bool, smaller integers,
native integers, byrefs/pointers, wide values, floating values and structs keep
their previous protocols. In particular an enum can have the same relaxed C++
`int32_t` type without qualifying for packed returns. Reducing wrapper size is
not the primary claim: the Wasm ABI probe verifies a direct `i64` return instead
of the legacy hidden result address. Other native ABIs need measurement rather
than assuming that a C++ wrapper always returns in registers.

## Calls and compatibility

The registration protocol field is now a flags word: bit 0 selects existing
void/reference returns, bit 1 enables eligible packed integer returns. Values
0/1/2/3 are supported, and unknown bits are rejected before registration. The
existing method binding caches these flags; no method/registration structure
size changes are introduced.

Declarations, direct calls, generic bodies, invokers and virtual adjustment
thunks use the same return policy. Dynamic bridges select the actual target's
flag and generic context, converting between legacy and packed representations
as needed. Runtime icall/intrinsic function signatures are explicitly kept
legacy; their results are adapted at the AOT boundary.

Bridge deduplication includes packed-integer eligibility even in legacy callers.
Otherwise an enum bridge and an `int` bridge could share a name despite needing
different dispatch branches. Reflection and interpreter calls keep ordinary
stack-buffer invoker interfaces. Exception handling retains normal-path AOT
and interpreter handoff, with no replay of earlier side effects.

Supporting foreign packed targets adds a target-protocol check to newly
generated eligible integer bridges even when the caller uses legacy returns;
separating enum/integer bridge identities can also increase bridge count.
Both sides of this round's benchmark include those changes. Their default-mode
cost relative to the parent commit is not separately isolated, so this trial
does not establish a zero-overhead default-mode upgrade.

**Rebuild the runtime and regenerate/rebuild every AOT caller and target before
mixing this new protocol.** An old runtime rejects flags 2/3, but old generated
integer bridges do not know how to call a packed target. Unchanged struct layout
does not make those old generated callsites compatible. Newly generated legacy,
lightweight and packed methods can interoperate through the updated bridges.

## Validation and reproduction

Seven new formal `TC_Int32Returns` cases cover signed/unsigned boundaries,
cross-assembly generic calls and reflective invocation, enum/interface dispatch,
divide/overflow errors, no-replay EH handoff, interpreter/icall boundaries,
excluded return types and delegates. They run alongside 24 generic and 14
interface cases, with native-side backend and handoff checks.

The standalone result tests check boundary and 4,096 pseudorandom payload bit
patterns, all 255 nonzero error-byte encodings, legacy conversions and nested
result independence. This is not an exhaustive traversal of 2^32 payloads.
Native hosts inject all four target flag combinations into signed/unsigned,
enum and generic interface implementations, including OOM materialization.
The OOM check injects an error code; it is not actual heap-exhaustion stress.

A dnlib fixture adds ABI-neutral `modopt(IsConst)` return modifiers to real
signed/unsigned interface implementations whose interface declarations remain
plain integers. The integer policy removes modifiers just as C++ type generation
does, while retaining enum/byref distinctions. Actual managed calls exercise
these modified implementations; injection tests alone would not prove that
their original generated bodies have the right ABI.

The matrix runner archives logs and reports before reusing build directories:

```powershell
python scripts/test/int32-returns.py --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_int32_returns.py --abi-only --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_int32_returns.py --build-only --emsdk D:/workspace/wasmclr/emsdk
# Wait for other compilation/tests to finish before sampling.
python scripts/benchmarks/compare_int32_returns.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

Wasm Release and Debug IR probes confirm `i64` packed signed/unsigned returns
without `sret`; the legacy integer probe retains `sret`. Native timing and code
size are measured separately. The benchmark compiles one managed DLL and copies
it byte-for-byte to the second build. Interface caches are disabled, and only
the integer return flag differs. Two new signed/unsigned chains retain work
after each noinline call. The older call/generic workloads may also benefit
from tail-call/inlining changes and cannot isolate the return-address effect.
The existing exception workload hands off to the interpreter after its first
throw; it does not measure accelerated native exception handling.

## Validation results

The primary matrix passed **72 build configurations / 96 CTest executions**.
Flags 0/1 cover native/Wasm Release controls; flags 2/3 additionally cover x64
Debug and Win32 Release. It includes generic discovery on/off, EH normal-path
and interpreted-loop controls, standalone result boundaries, and two-entry
interface caches combined with packed returns. Twenty-four formal configurations
each execute 45 methods, totaling **1,080 formal method executions**, including
168 new integer case executions. Runtime regressions passed **41/41**.

After finding the custom-modifier boundary, an additional sixteen generic
fixture configurations (all four protocols, discovery on/off, native/Wasm
Release) passed, including 32 CTest executions. These follow-up checks use
modified signatures; the primary Debug/Win32 checks use the ordinary signatures.
Final generation of the benchmark with modifier normalization produces
byte-identical C++ to the built benchmark sources, recorded in the provenance.

CI now checks four protocol modes and Release/Debug Wasm ABI probes. Local runs
were Windows/Node; remote CI, Linux/ARM, browsers and the full managed/GC suites
were not rerun this round. Earlier performance snapshots remain unchanged.

## Measured results and decision

MSVC 19.44.35229.0, Emscripten/Clang 22.0.0, Node v22.12.0. Each workload runs
5,000,000 iterations in seven interleaved process samples. No samples were
discarded; CPU frequency and all background activity were not controlled.
Both builds keep void/reference returns legacy and disable interface caches.
The table reports ns/iteration, **not cost per individual CLR call**; each new
chain contains three noinline method calls and post-call work.

| Workload | x64 legacy | x64 packed | Wasm legacy | Wasm packed |
| --- | ---: | ---: | ---: | ---: |
| Signed non-tail chain | 3.573 | 3.195 | 5.022 | 3.214 |
| Unsigned non-tail chain | 3.568 | 3.182 | 5.017 | 3.176 |
| Interface loop | 5.870 | 6.033 | 7.039 | 6.713 |
| Array control | 0.792 | 0.794 | 0.795 | 0.801 |
| Void-return chain control | 2.175 | 2.181 | 3.450 | 3.386 |
| Reference-return chain control | 1.499 | 1.548 | 2.896 | 2.902 |

Signed/unsigned chain medians fall **36.0%/36.7% on Wasm**, and **10.6%/10.8%
on x64**, with nonoverlapping sample ranges for all four comparisons. Interface
and unchanged control ranges overlap; they do not establish a general benefit.
The older calls/generic workloads show larger Wasm changes but include other
compiler optimizations, so they are not the primary evidence. The struct
workload also calls an integer-returning helper: any improvement there does
not mean struct-return ABI support has changed. All twelve workloads and their
checksums/ranges are retained in the raw data.

| Artifact bytes | Legacy | Packed | Difference |
| --- | ---: | ---: | ---: |
| x64 executable | 887296 | 886272 | -1024 |
| Wasm file | 838207 | 836825 | -1382 |
| Wasm code section | 672216 | 670832 | -1384 |
| Wasm gzip | 267925 | 267847 | -78 |

These sizes include the shared test runtime and exclude external BCL/IL/resources.
They show a small fixture-level reduction, not a large game/deployment saving.
No runtime-wide memory reduction or actual OOM stress result is claimed.

Keep the new flag **off by default**, available as an explicit Wasm-oriented
trial. The evidence supports these integer chains; it does not justify a global
runtime Result replacement or automatic enablement on every platform. Complete
parent-commit default-mode regression measurement, browser/real-application
workloads and native ABI analysis remain separate work before broader rollout.

Raw samples, IR signatures, build/toolchain commands, byte-identical managed
input hashes, source/generated hashes, artifact sizes and validation evidence
are in [aot-int32-returns.json](data/aot-int32-returns.json).
