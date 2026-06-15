# Scripts

Repository-level build, test, and developer scripts. Module-specific scripts (samples, aot-tester) remain next to their projects.

All build and intermediate outputs go under **`out/`** at the repository root (override with `LEANCLR_OUT_ROOT`). Source trees under `src/` stay clean.

```text
out/
├── cmake/<module>/<Config>-<Arch>/   # CMake build trees & native binaries
└── dotnet/<ProjectName>/<Config>/    # .NET assemblies (see Directory.Build.props)
```

Clean all outputs: `scripts\dev\clean-out.bat` (Windows) or `./scripts/dev/clean-out.sh` (Unix).

## Quick start

| Task | Linux / macOS | Windows |
|------|---------------|---------|
| Build & run all tests | `./scripts/ci.sh Release` | `scripts\test\build-all.bat Release` then `scripts\test\run.bat Release` |
| Build tests only | `./scripts/build.sh test build Release` | `scripts\build.bat test build Release` |
| Run tests | `./scripts/build.sh test run Release` | `scripts\build.bat test run Release` |
| Build runtime | `./scripts/build.sh runtime Release` | `scripts\build.bat runtime Release` |
| Build LeanAOT | `./scripts/build.sh leanaot [Debug\|Release]` | `scripts\build.bat leanaot Debug` or `Release` |
| Build aot-tester | `./scripts/build.sh aot-tester [Config] [Arch]` | `scripts\build.bat aot-tester [Config] [Arch]` |
| Generate aot-tester C++ | `./scripts/build.sh aot-tester gen-cpp` | `scripts\build.bat aot-tester gen-cpp` |
| Generate aot-tester C++ (posix) | `./scripts/build.sh aot-tester gen-cpp-posix` | `scripts\build.bat aot-tester gen-cpp-posix` |
| Run aot-tester | `./scripts/build.sh aot-tester run [Config] [Arch]` | `scripts\build.bat aot-tester run [Config] [Arch]` |
| Build aot-tester (wasm) | `./scripts/build.sh aot-tester build-wasm [Config]` | `scripts\build.bat aot-tester build-wasm [Config]` |
| Format runtime C++ | — | `scripts\dev\format-cpp-files.bat` |
| Clean outputs | `./scripts/dev/clean-out.sh` | `scripts\dev\clean-out.bat` |

Legacy paths under `src/tests/` forward to `scripts/test/` for compatibility.

## Layout

```
scripts/
├── build.sh / build.bat     # Orchestration entry point
├── ci.sh                    # CI: build + run tests (Linux)
├── test/                    # Unit test build & run
├── runtime/                 # leanclr runtime library (CMake)
├── leanaot/                 # LeanAOT tooling (aot-tester build, …)
├── generator/               # Opcode / icall code generation
├── dev/                     # clean-out, format-cpp-files
└── lib/                     # repo-root, out-dir-init, cmake-dir
```

## Script index

| Script | Description |
|--------|-------------|
| `ci.sh` | CI entry: `test/build-all` + `test/run` |
| `test/build-all.*` | Build C++ runner + managed test DLLs → stage to runner `dlls/` |
| `test/basic-tester/build.*` | Build C++ test runner executable |
| `test/run.*` | Run `out/cmake/tests/basic-tester/<Config>-<Arch>/bin/.../test` |
| `runtime/build.*` | Build `leanclr` → `out/cmake/runtime/<Config>-<Arch>/` (Windows: optional `sln`, `shared`) |
| `test/aot-tester/build.*` | Build native `aot-tester` executable |
| `test/aot-tester/gen_cpp.*` | Run LeanAOT to generate C++ under `src/tests/aot-tester/cpp/` |
| `test/aot-tester/gen_cpp_posix.*` | Generate C++ under `src/tests/aot-tester/cpp-posix/` using POSIX BCL (`dotnetframework4.x-linux`) |
| `test/aot-tester/run.*` | Build and run `aot-tester` with `AotTests` assembly |
| `test/aot-tester/build-wasm.*` | Build `aot-tester` with Emscripten → `out/cmake/tests/aot-tester/<Config>-wasm/` |
| `leanaot/build.*` | Build LeanAOT → `out/dotnet/LeanAOT/<Config>/net8.0/` |
| `publish_leanaot.bat` | `dotnet publish` Release → `../leanclr-unity/LeanCLR~/leanaot/` (sibling repo) |
| `publish_runtime.bat` | Mirror `src/runtime` → `../leanclr-unity/LeanCLR~/runtime/` (excludes scripts, CMakeLists.txt, `3rd/zlib`) |
| `generator/gen_*.bat` | Regenerate opcode headers and icall JSON |
| `dev/format-cpp-files.bat` | clang-format `src/runtime` sources |
| `dev/clean-out.*` | Remove entire `out/` directory |

## Co-located scripts

These invoke CMake/dotnet but write outputs only under `out/`:

- `src/runtime/build.*` — forward to `scripts/runtime/`
- `src/samples/*/build.*` — platform samples
- `src/tests/basic-tester/build.*` — forward to `scripts/test/basic-tester/`
- `src/tools/lean/build.*` — lean tool
