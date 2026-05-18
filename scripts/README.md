# Scripts

Repository-level build, test, and developer scripts. Module-specific scripts (samples, aot-runner) remain next to their projects.

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
| Build aot-runner | — | `scripts\build.bat aot-runner [Config] [Arch]` |
| Generate aot-runner C++ | — | `scripts\build.bat aot-runner gen-cpp` |
| Generate aot-runner C++ (posix) | — | `scripts\build.bat aot-runner gen-cpp-posix` |
| Run aot-runner | — | `scripts\build.bat aot-runner run [Config] [Arch]` |
| Build aot-runner (wasm) | — | `scripts\build.bat aot-runner build-wasm [Config]` |
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
├── leanaot/                 # LeanAOT tooling (aot-runner build, …)
├── generator/               # Opcode / icall code generation
├── dev/                     # clean-out, format-cpp-files
└── lib/                     # repo-root, out-dir-init, cmake-dir
```

## Script index

| Script | Description |
|--------|-------------|
| `ci.sh` | CI entry: `test/build-all` + `test/run` |
| `test/build-all.*` | Build C++ runner + managed test DLLs → copy DLLs to `src/tests/managed/MiscDlls/` |
| `test/run.*` | Run `out/cmake/tests/basic_test_runner/<Config>-<Arch>/bin/.../test` |
| `runtime/build.*` | Build `leanclr` → `out/cmake/runtime/<Config>-<Arch>/` (Windows: optional `sln`, `shared`) |
| `leanaot/aot-runner/build.bat` | Build native `aot-runner` executable |
| `leanaot/aot-runner/gen_cpp.bat` | Run LeanAOT to generate C++ under `src/leanaot/aot-runner/cpp/` |
| `leanaot/aot-runner/gen_cpp_posix.bat` | Generate C++ under `cpp-linux/` using POSIX BCL (`dotnetframework4.x-linux`) |
| `leanaot/aot-runner/run.bat` | Build and run `aot-runner` with Test assembly |
| `leanaot/aot-runner/build-wasm.bat` | Build `aot-runner` with Emscripten → `out/cmake/leanaot/aot-runner/<Config>-wasm/` |
| `leanaot/build.*` | Build LeanAOT → `out/dotnet/LeanAOT/<Config>/net8.0/` |
| `publish_leanaot.bat` | `dotnet publish` Release → `../leanclr4unity/LeanCLR~/leanaot/` (sibling repo) |
| `publish_runtime.bat` | Mirror `src/runtime` → `../leanclr4unity/LeanCLR~/runtime/` (excludes scripts, CMakeLists.txt, `3rd/zlib`) |
| `generator/gen_*.bat` | Regenerate opcode headers and icall JSON |
| `dev/format-cpp-files.bat` | clang-format `src/runtime` sources |
| `dev/clean-out.*` | Remove entire `out/` directory |

## Co-located scripts

These invoke CMake/dotnet but write outputs only under `out/`:

- `src/runtime/build.*` — forward to `scripts/runtime/`
- `src/samples/*/build.*` — platform samples
- `src/leanaot/aot-runner/*.bat` — forward to `scripts/leanaot/aot-runner/` (generated `cpp/` still under aot-runner until phase 3)
- `src/tests/basic_test_runner/build.*` — test runner executable
- `src/tools/lean/build.*` — lean tool

