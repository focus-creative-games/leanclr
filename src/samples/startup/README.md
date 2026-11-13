# Startup sample

`startup` is the only sample in this repository. It configures an assembly loader,
initializes the runtime, and reports that the matching CoreLib loaded. The
`src/tools/leanrun` tool handles managed program execution. Engine integrations
live in the separate LeanCLR Unity, Godot, and UE repositories linked in the root
README.

## Build

From the repository root, with CMake, a C++17 compiler, and a compatible BCL:

```powershell
cmake -S src/samples/startup -B out/cmake/samples/startup/Release-x64 -A x64
cmake --build out/cmake/samples/startup/Release-x64 --config Release
```

The default BCL adapter is Mono. On `coreclr-minimal`, add
`-DLEANCLR_BCL_ADAPTER=CORECLR_NET10` to configure for the .NET 10 BCL. The BCL must
match the runtime adapter and target pointer width.

From this sample directory, `build.bat Release x64` or
`bash build.sh Release` builds the default adapter. `generate-vs-sln.bat x64`
generates a Visual Studio solution. Generated files go under the repository's
`out/cmake/samples/startup` directory; the scripts print the exact paths.

## Run

The sample searches the current directory first. Run the executable from a
folder containing the matching BCL:

```powershell
# Replace the path with the BCL selected above.
Set-Location <matching-bcl-directory>
& <repository>/out/cmake/samples/startup/Release-x64/bin/Release/startup.exe
```

A successful run prints `Corlib assembly loaded successfully.` On Linux/macOS,
omit `-A x64` when configuring, add `-DCMAKE_BUILD_TYPE=Release`, and run
`out/cmake/samples/startup/Release-x64/bin/startup` from the BCL folder.

## Validation

Maintained coverage lives under `src/tests`, including `public-invoker` for real
C callbacks, PInvoke registration, exception handoff, and pointer-width contracts.
The runtime CI executes it on native and Wasm targets. The C++17 validation script
also builds this startup sample for native and Wasm, without browser packaging.

Wasm JavaScript PInvoke implementations remain in
`src/tests/managed/AotTests/leanclr_test_pinvoke.js`, linked by the AOT tester.
