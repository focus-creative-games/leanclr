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
