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
