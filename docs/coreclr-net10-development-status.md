# Minimal .NET 10 development status

Updated 2026-11-11 on the self-owned `coreclr-net10` branch. Implementation work and target acceptance are recorded separately. This is a minimal runtime, not full Microsoft.NETCore.App compatibility. **Platform/SDK acceptance is not fully closed:** Linux, ARM64, UBSan and actual Godot/Unreal SDK runs were unavailable locally.

| Migration stage | Delivered implementation and commits | Local evidence / remaining acceptance |
| --- | --- | --- |
| P0 baseline | `421aecfd`, `ed2a399b`, `01cf2402`: native binding registration, owned command-line storage, RuntimeHelpers/static-interface/IR stack propagation and normalized narrow native returns | Both original failures pass. Windows x64 Release/Debug: 77 smoke entries, 15 GC/EH entries, 6 API entries and 3 native contracts. No native crash or assertion dialog in those runs. Default Mono native 41/41 and generic/EH AOT regressions pass. |
| P0 CoreLib ABI / handles | `ec2ba9ed`, `0a330968`, `bfd282e9`: field name/type/offset/size gates, canonical type/native metadata identities, two-pointer TypedReference, ctor tokens, inline-array layouts, target-width CoreLib rejection; remove unsupported fixed-width ArgIterator writes | typeof/GetType, arrays, generics, reflection, typedref and invalid native identities pass on native/Wasm; mutated layouts and foreign pointer widths are explicitly rejected. Every claimed supported native CoreLib view is gated; unsupported APIs are not authorized by a type/assembly name. |
| P0 GC / EH | `118fb8a1`, `8d220763`, `ad0c8624`: live interpreter/native/AOT roots, byref/value references, conditional dependent handles, interior range index, finalizer closure, exception state and try-boundary fixes | 15 real-heap/EH cases pass against stock .NET 10 and inside the 98-entry native ASan/Wasm AOT gates. Nested catch/finally, filter/rethrow, byref replacement and reflection arguments have regressions. UBSan remains a pending Linux CI acceptance gate. |
| P1 minimal API inventory | `1fb47add`, `98f3e650`, `bfd282e9`: schema-2 validated/unsupported/pending inventory, array/Span/reflection/string/collection contracts, explicit rejected worker starts, varargs and ArgIterator cookies | 6 API groups pass on native/Wasm. Final historical inventory remains **434/459**; the 25 failure names are unchanged and published with stock-runtime comparisons in the API coverage document. These are explicit compatibility gaps, not a green full-framework gate. |
| P1 net10 AOT | `ad0c8624`, `98f3e650`, `0a330968`: bindings, handle ABI, root snapshots, inline-array/sizeof, cached static interfaces, shared generic and return protocols; explicit vararg fallback manifest | **98/98** default entries under Windows MSVC ASan and **98/98** optimized entries; Wasm32 **98/98** default and optimized. Net10 and Mono specialized generic/disabled/return-ABI/EH harnesses pass natively and in Node. Snapshot EH counters prove normal side effects are not replayed. |
| P1 platforms / budgets | `2a87ca9d`, `0a330968`, `fa168e23`: bounded facades/layout flag cache, exact slot address index, portable bindings/build gates, width-correct QCall aggregate reads, isolated native/Wasm resource comparison | Windows and Wasm local gates pass; native and Wasm resource/handle contracts **3/3**. Five-run before/after times, module size, live heap and peak memory are published. CI covers Linux x64/ARM64 and ASan+UBSan, but **those remote jobs have not been run here**. WSL's registered ext4.vhdx is missing; no local Docker/ARM64 execution host was available. |
| P2 minimal engine host | `f47451c2`: self-owned C ABI, owned file buffers/limits, terminal lifecycle, thread/reentry and fixed error contracts; independent Godot/Unreal lifecycle owners | **14/14** cases in Windows Debug, ASan, shared-DLL Release and Node Wasm32. C header compiles as C; native tests use an actual wrong-thread call. Engine tests are separate from runtime tests. **Real Godot/Unreal SDK registration, packaging and editor launch are not validated**; SDK-neutral hook tests are not counted as engine launches. |

## Reproduce

```powershell
./scripts/dotnet10/integration-baseline.ps1 -BuildDir out/net10-integration -ContractsBuildDir out/net10-contracts -AllSmokeEntries
./scripts/dotnet10/integration-baseline.ps1 -BuildDir out/net10-integration -ContractsBuildDir out/net10-contracts -AllSmokeEntries -Configuration Debug
python scripts/dotnet10/integration.py
python scripts/dotnet10/aot-contracts.py --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8"
python scripts/dotnet10/aot-contracts.py --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8" --variant optimized
python scripts/dotnet10/fetch-corelib.py
python scripts/dotnet10/aot-contracts.py --target wasm --runtime out/net10-runtime-win-x86/runtimes/win-x86/lib/net10.0 --emsdk D:/workspace/wasmclr/emsdk
python scripts/dotnet10/aot-contracts.py --target wasm --variant optimized --runtime out/net10-runtime-win-x86/runtimes/win-x86/lib/net10.0 --emsdk D:/workspace/wasmclr/emsdk
python scripts/dotnet10/engine-contracts.py --configuration Debug
python scripts/dotnet10/engine-contracts.py --shared
```

ASan requires its MSVC runtime DLL directory on PATH and `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1:allocator_may_return_null=1`; use `aot-contracts.py --configuration RelWithDebInfo --asan` and `engine-contracts.py --configuration RelWithDebInfo --asan`. Process-lifetime metadata is not an unload/leak-free shutdown claim. Native Unix CI uses Clang `--sanitizers address,undefined` instead.

Local toolchain: .NET SDK 10.0.204, CoreLib 10.0.8, MSVC 14.44.35207, Emscripten 4.0.12, Node 22.12.0. CoreLib SHA256:

- Windows x64: `5B6700E2802AFDB2CC4863B096760A66442E89FD6EE5EDB718C697AE5CF33DDC`.
- Wasm's x86 CoreCLR BCL: `E0457D3EBFF7AD8F87534D0163786C66F623B7118F2E82F22FA0E8B6D8F61A28`.

Detailed evidence and boundaries: [API coverage](coreclr-net10-api-coverage.md), [AOT contract](coreclr-net10-aot-contract.md), [resource budget](coreclr-net10-resource-budget.md), [engine hosting](coreclr-net10-engine-host.md). `net10-platforms.yml` supplies Windows, Linux/ARM64/sanitizer and Wasm jobs and uploads their evidence when executed. All development commits use English; Core/Mono/third-party branch tips are unchanged, reports remain locally excluded, and nothing was pushed.
