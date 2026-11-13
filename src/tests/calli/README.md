# calli contracts

Stage 03-A validates managed target provenance before dereference, full return/parameter
signatures, byref and instance flags. MethodHandle.GetFunctionPointer exports the same
managed descriptor as ldftn; Marshal/registered native callbacks use a separate native
ABI protocol. Arbitrary native addresses are never treated as metadata descriptors.

The index grows fallibly and records only escaped methods. Native tests cover invalid
addresses, different parameter counts/types/returns, invocation-convention metadata,
10,000 allocation-free repeat registrations/validations, and failed growth/retry.
Positive managed IL is compared against .NET 10. BadTarget/NullTarget/WrongSignature
are LeanCLR guard contracts and must not be run as arbitrary native calls on stock CLR.

Build CalliTests.csproj and configure this CMake directory with LEANCLR_CALLI_MANAGED_DIR
and LEANCLR_CALLI_RUNTIME_DIR. For Mono compile Program.cs with modern Roslyn using
-unsafe -target:library -nostdlib against mono-4.5/mscorlib.dll, and select MONO.

Stage 03-B adds managed calli AOT via the uniform invoker, preserving actual generic
caller contexts and mixed AOT/interpreter targets. A per-site fast cache avoids
reparsing signatures on warm calls. Runtime and generated roots protect arguments,
locals and evaluation-stack references across collection. Mono also scans its live
interpreter frames through the checked heap-owner index.

The dnlib fixture tool emits real instance, explicit-this, virtual, value-this and
boxed virtual IL. Its optional `verify` argument runs those five cases on .NET 10.
Configure LEANCLR_CALLI_AOT_CPP_DIR to link generated bodies into calli-native;
the harness asserts that tested entry points really use AOT invokers.
GC cases check payload contents after allocation pressure, not just pointer equality.
The EH case verifies that normal-path side effects are not replayed after handoff.

Unmanaged calls remain rejected until signature bridges are linked and supported.

Stage 03-C generates `calli_bridges.cpp` and `calli_bridges.json` by scanning all
input IL, including excluded methods, and instantiated generic plans. Use
`--calli-bridges-only` with the usual -a/-d/-o arguments to build signature bridges
without compiling any managed method bodies. Link that file into the host and call
`Calli::register_native_bridges(g_calli_bridges, g_calli_bridge_count)`; normal AOT
module registration does this automatically. Tables have runtime lifetime.

The runtime validates compiler-derived ABI shapes and hashes the complete shape,
then compares collision candidates in full. Repeat registrations are allocation-free;
a missing bridge reports its convention, return and parameter ABI shapes. Supported
platform gates currently cover Windows x64 and Emscripten wasm32 only.

Stage 03-D splits CalliNative IR from CalliInterp and caches the bound native
bridge at translation. Generated AOT uses the same site protocol. Native tests
obtain real C function addresses from native_fixture.c; they cover mixed floating
point/integer ABI, twelve arguments, ref/out, narrow signed/unsigned results,
pointers, native integers and null/managed-descriptor rejection. The shared fixture
can also be loaded by `dotnet CalliTests.dll Unmanaged <fixture.dll>`.

.NET 10 Windows comparison confirmed default calli bool is a 32-bit BOOL, while
char is an ANSI byte with character conversion. Bool bridges normalize nonzero
returns; bool/char byrefs need separate marshaling and currently reject. ANSI char
is enabled only on Windows and compared using the NativeChar case. Use ushort for
an explicit 16-bit native value; Wasm char remains unsupported. SuppressGCTransition,
MemberFunction, varargs and unknown/required convention modifiers remain rejected.

Stage 03-E adds recursively checked blittable aggregate shapes. The table records
C++ sizeof/alignof/offsetof, nested field types and offsets. Binding compares those
against initialized runtime layouts. Tests cover 8-byte and 24-byte returns, float
and double aggregates, nested ref values, Pack=1, explicit padding and overlapping
union fields; the same C functions are compared on Windows .NET 10 and Wasm.

AutoLayout, managed-reference fields, MarshalAs conversions, bool/char fields,
SIMD/inline-array special types and unverified alignments are rejected. Shape
recursion is limited to 16 and the per-bind nested-field budget to 128. Cold binding
uses bounded scratch storage; hot calls retain just the cached immutable bridge.

Stage 03-F adds repeated native-to-managed collection, interior/evaluation-stack
roots, EH side-effect checks, transactional bridge OOM/retry, and paired benchmarks
against a statically linked named P/Invoke. The named and calli loops call the same
non-inlined C function; the harness checks equal results and no warm native allocations.
`verify-native.ps1 -Benchmark` builds fixtures, checks bridge dedup/closed generic
census, runs interpreter and actual AOT, and compares positive cases on .NET 10.

Managed reference variance is supported; the interpreter caches its last validated
target. Null-this calli uses the interpreter when its AOT body assumes non-null this.
ABI-only guard/target code lives separately to prevent accidental runtime linking.
Use LEANCLR_CALLI_SIZE_PROBES=ON for paired size probes (whole bridge table + guard,
not a single bridge). Unsupported ANSI char wrappers are omitted on portable hosts.

Linux x64/ARM64 CI jobs use the explicit LEANCLR_CALLI_ABI_PROBE validation flag.
Their production capability stays disabled until actual ABI results are reviewed.
All caches currently follow the runtime's SINGLE-thread ownership contract.
See docs/calli-support-results-2026-11-13.md for measured results and limits.
