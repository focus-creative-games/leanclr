# Structure marshalling contracts

The same `Program.cs` is compiled against Mono mscorlib and .NET 10 CoreLib.
`Program.All` runs 21 differential cases without using Console, so it also runs
with the portable host. `FailureCleanup` adds a stronger transactional-write
contract; `Pressure` exercises allocation and collection repeatedly.

The shared native-layout engine covers sequential and explicit formatted types,
packing, explicit size, nested value types, primitives/pointers and enum fields, BOOL and
VARIANT_BOOL, ANSI/Unicode characters, LPStr/LPWStr/LPUTF8Str/BStr, ByValTStr,
and fixed ByValArray fields (including string and nested structure elements).
Native string ownership uses the runtime's existing Marshal allocation family;
external owners must use matching allocation/free functions.
SizeOf and OffsetOf use the same native layout as the converters. Formatted
classes support existing-object reads and parameterless-constructor allocation.
DestroyStructure releases field resources without freeing the outer buffer;
StructureToPtr with deleteOld=true replaces resources only after a successful
conversion. Explicit unions with overlapping converted fields are rejected;
blittable unions are supported.

COM/interface/custom marshalers, delegate fields, variable-length LPArray,
SafeArray, and generic formatted types are outside this implementation. Invalid
or unsupported layouts fail before native writes rather than exposing managed
references as native pointers. ANSI uses the selected host ABI (Windows ACP or
UTF-8 for portable/POSIX hosts).

## Build and run

Build the .NET 10 fixture:

```powershell
dotnet build src/tests/struct-marshal/StructMarshalTests.csproj -c Release -o out/struct-marshal/net10
dotnet out/struct-marshal/net10/StructMarshalTests.dll
```

For Mono, compile `Program.cs` as `StructMarshalTests.dll` with Roslyn csc,
using `-nostdlib` and referencing the repository's Mono `mscorlib.dll`.

Configure the CTest runner with the matching managed fixture and BCL directories:

```powershell
cmake -S src/tests/struct-marshal -B out/struct-marshal/contracts -A x64 `
  -DLEANCLR_BCL_ADAPTER=CORECLR_NET10 `
  "-DLEANCLR_MARSHAL_MANAGED_DIR=<absolute fixture directory>" `
  "-DLEANCLR_MARSHAL_RUNTIME_DIR=<matching CoreLib directory>"
cmake --build out/struct-marshal/contracts --config Release --parallel 4
ctest --test-dir out/struct-marshal/contracts -C Release --output-on-failure
```

Use BCL adapter `MONO` and host `PORTABLE` to verify mono-minimal; use host
`NATIVE` for mono-full. For Wasm, use the Emscripten toolchain and a **32-bit**
CoreLib. The test Node host mounts the repository at `/host`; configure fixture
and BCL paths under that mount. Keep those inputs inside the repository and
quote directory arguments such as `/host/src/libraries/mono-4.5`.

`native.cpp` also decodes real LPArray, SafeArray and custom-marshaler metadata
and checks that all fields in each tagged payload survive subsequent reads.

`native.cpp` tracks allocations from before runtime startup. It verifies that
10,000 cached blittable writes perform no allocations; repeated replacement
has stable ownership; each staged native allocation can fail without leaking
resources or mutating the previous native value; destruction releases the owned
strings. Test bookkeeping intentionally outlives runtime global destructors.

The packed `Text` and nested text fixtures also cover real AOT callers. Generated
managed structs containing references must use the runtime's natural GC-slot
alignment, independently of their packed native Marshal representation.
