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
| Publish LeanAOT | `./scripts/build.sh leanaot publish` | `scripts\build.bat leanaot publish` |
| Format runtime C++ | — | `scripts\dev\format-cpp-files.bat` |
| Clean outputs | `./scripts/dev/clean-out.sh` | `scripts\dev\clean-out.bat` |

Legacy paths under `src/tests/` forward to `scripts/test/` for compatibility.

## Layout

```
scripts/
├── build.sh / build.bat     # Orchestration entry point
├── ci.sh                    # CI: build + run tests (Linux)
├── test/                    # Unit test build & run
├── release/                 # Publish LeanAOT, etc.
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
| `release/publish-leanaot.*` | `dotnet publish` LeanAOT to `src/tools/leanaot/` |
| `generator/gen_*.bat` | Regenerate opcode headers and icall JSON |
| `dev/format-cpp-files.bat` | clang-format `src/runtime` sources |
| `dev/clean-out.*` | Remove entire `out/` directory |

## Co-located scripts

These invoke CMake/dotnet but write outputs only under `out/`:

- `src/runtime/build.*` — runtime library
- `src/samples/*/build.*` — platform samples
- `src/leanaot/aot-runner/*.bat` — AOT pipeline (generated `cpp/` still under aot-runner until phase 3)
- `src/tests/basic_test_runner/build.*` — test runner executable
- `src/tools/lean/build.*` — lean tool
