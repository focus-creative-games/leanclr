# Runtime regression tests

These tests cover the B01–B13 review fixes: empty and cloned multidimensional arrays, bounded metadata, exported types, failed initializers, invocation allocation ownership, weak handles, allocation failure, loader callbacks, GC accounting, compressed integers, public ABI consumers, and integer execution semantics.

Prepare the BCL and managed DLLs using the repository test build scripts, then run from the repository root:

```powershell
cmake -S src/tests/runtime-regressions -B out/regressions -A x64 -DCMAKE_TRY_COMPILE_CONFIGURATION=Release
cmake --build out/regressions --config Release
ctest --test-dir out/regressions -C Release --output-on-failure
```

Set `LEANCLR_TEST_DLL_DIR` to an alternative DLL staging directory. Fixture tests require `csc` or `mcs`; the module initializer test requires `ilasm`. CMake discovers the Windows Framework tools by default. `runtime-managed-tests` is the existing managed test runner linked against the same runtime; copy the staged DLL directory beside it as `dlls` before running it.

For shared-library ABI verification, configure a separate build with `-DBUILD_SHARED_LEANCLR=ON -DLEANCLR_REGRESSION_PUBLIC_API_ONLY=ON`. This builds C and C++ consumers of the exported API. CMake propagates `LEANCLR_STATIC` for static consumers; non-CMake static consumers must define it themselves. The C++ reference overloads remain available, while the exported functions use C pointer parameters.

To run the same fixture tests through generated AOT code, first build LeanAOT, generate the fixture sources, and reconfigure with their absolute path:

```powershell
dotnet build src/leanaot/LeanAOT/LeanAOT.csproj -c Release
dotnet out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll -d out/regressions -d src/libraries/mono-4.5 -a RegressionFixtures -o out/regression-aot --leanaot-aot-rule-file src/tests/runtime-regressions/aot-rules.xml
cmake -S src/tests/runtime-regressions -B out/regressions -DLEANCLR_REGRESSION_AOT_DIR=D:/absolute/path/to/out/regression-aot
cmake --build out/regressions --config Release
ctest --test-dir out/regressions -C Release --output-on-failure
```

The integer execution test checks that its methods actually use AOT invokers when AOT is enabled. Clear `LEANCLR_REGRESSION_AOT_DIR` to restore interpreter execution. Regenerate AOT sources whenever `RegressionFixtures.cs` changes.

The standalone `integer-math.cpp` also supports compiler sanitizer checks without runtime initialization, for example Clang `-O2 -fsanitize=undefined -fsanitize-trap=undefined`. The tests check wraparound and shifts for 32-bit, 64-bit, and native integers, including the signed minimum and randomized arithmetic.

Allocation failure tests inject failure into host allocation callbacks after metadata is warmed. Fallible object, array, handle, string icall and invocation paths return `OutOfMemory`; raw string allocation helpers return `nullptr`. `Vector::try_reserve` and `try_push_back` preserve the container on failure. Existing infallible metadata/container operations have no recoverable return channel and terminate through `panic`/`abort` on allocation failure.

GC used/heap byte counters describe occupied allocation blocks, including size-class rounding and multidimensional bounds, rather than retained free arena capacity. The lifetime allocation counter reports aligned object allocation bytes and is not reset by collection. Core still requires the host to control GC timing outside managed execution.
