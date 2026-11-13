# Reflection invocation contracts

The same managed cases run against stock .NET 10, CoreCLR minimal, and both Mono profiles.
They cover value/ref/out copy-back, original box aliasing, null inputs, nullable values,
enums, structs containing GC references, return values, collection during calls, and
failure without copy-back. The minimal runtime currently propagates the target exception
directly; these tests check argument state independently of reflection exception wrapping.

Small calls keep argument, return, and byref bookkeeping inline. Value byrefs use fresh
GC-managed boxes, avoiding a native buffer plus a later boxing allocation. Nullable
outputs are reserved before the call, so successful copy-back cannot fail halfway through.
Only successful invocation commits parameter updates. Root scopes cover conversion,
class initialization, invocation, return boxing, and nested reflection calls.

Example (PowerShell, from the repository root):

```powershell
dotnet build src/tests/reflection-invoke/ReflectionInvokeTests.csproj -c Release -o out/reflection-invoke/managed
cmake -S src/tests/reflection-invoke -B out/reflection-invoke/native -A x64 `
  -DLEANCLR_BCL_ADAPTER=CORECLR_NET10 `
  -DLEANCLR_REFLECTION_MANAGED_DIR="$PWD/out/reflection-invoke/managed" `
  '-DLEANCLR_REFLECTION_RUNTIME_DIR=C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8'
cmake --build out/reflection-invoke/native --config Release
ctest --test-dir out/reflection-invoke/native -C Release --output-on-failure
```

For Mono, compile Program.cs to ReflectionInvokeTests.dll with modern Roslyn using
`-target:library -nostdlib -r:<mono-4.5/mscorlib.dll>`, select the MONO adapter, and
point the two test directories at that output and the Mono BCL. The portable Wasm
runner must use a matching 32-bit CoreLib.

The native target directly exercises RuntimeMethodHandle_InvokeMethod QCall with value, reference, nullable, null-pointer, GC, and exceptional arguments. CoreCLR's indirect argument root scope retains all typed source slots while inputs are materialized, including reference fields inside unboxed structs. The QCall frontend copies changed byrefs into the supplied slots only after successful invocation.

The common native constructor contracts call the actual stackobject API on an existing
reference object and an unboxed value payload. They check original-object identity,
field updates, exact constructor count, one-time type initialization, and preservation
of the return buffer for void constructors. Object-based invocation with an existing
target is also distinguished from explicit allocation using a null target. The report's
claimed stackobject/newobj substitution did not reproduce on the current code.

A CoreCLR native regression also makes the first value argument's type initializer collect before a later raw struct argument is boxed. All parameter layouts must be prepared before that first managed allocation, so the later struct's GC references are visible throughout materialization.

For the raw native contract with actual compiled constructor bodies, generate
ReflectionInvokeTests with LeanAOT for the selected BCL, then configure
`LEANCLR_REFLECTION_AOT_CPP_DIR` with that generated directory. This links the AOT
module into the native harness and asserts that both reference/value constructors
have AOT invokers before checking the existing-this behavior; interpreter-only
execution cannot accidentally satisfy this gate.
