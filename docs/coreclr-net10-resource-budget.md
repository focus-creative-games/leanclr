# Minimal net10 resource budget

Windows x64 / MSVC Release and Node Wasm32 were measured locally. Each cell is the median of five separate processes, 100,000 lookups each, with 8,192 real managed arrays. The native baseline executable was saved before replacing linear slot lookup; this is not an IL2CPP comparison.

| Handles | Native linear slot, ns | Native indexed slot, ns | Wasm indexed slot, ns |
| --- | ---: | ---: | ---: |
| 1024 | 669.96 | 1.917 | 6.203 |
| 4096 | 2513.23 | 4.467 | 8.498 |
| 16384 | 9841.336 | 5.938 | 10.194 |

The slot address index validates exact active slots without dereferencing untrusted addresses, allocates fallibly before publication, erases freed entries and preserves slot stability through rehash. Its expected lookup is O(1), with O(n) storage; at 16,384 live handles the 32,768-entry address table costs 256 KiB on x64 / 128 KiB on Wasm32, in addition to the existing ID table. Hash collision worst cases remain O(n).

Interior membership uses the sorted arena range index introduced in the GC stage, O(log arenas), followed by constant-time small-block membership. At 16,384 handles the native valid/invalid membership pair median is 35.930 ns, Wasm 68.263 ns. Registration/removal shifts the cold range vector, O(arenas); no assembly/method scan occurs in these paths.

The native current peak working set is 29028352 bytes; Wasm linear memory reaches 24772608 bytes. Wasm linear memory is monotonic, so the final size is also its peak for this test. Native baseline peak RSS was not recorded and must not be inferred. Managed live heap is 2416280 native / 2283464 Wasm bytes. The native baseline had 1,472 fewer managed bytes because it predates the extra array-facade warmup and budget contract, not because handle indexing allocates managed objects.

Binary sizes: native baseline 1137152 bytes, current 1138688 bytes; Wasm module 997508 bytes, Node JS loader 71327 bytes. These are this resource executable, not a full application's compressed download size. A historical Wasm baseline binary is unavailable; no Wasm before/after size or performance claim is made.

RuntimeType facade registration defaults to 16,384 canonical identities and is configurable on the runtime owner thread through Settings / engine config. New identities fail with OutOfMemory at the cap, repeated failures do not grow the registry and existing identities remain valid. A combined, correctly aligned facade/auxiliary allocation reduces metadata allocation overhead. Layout-derived flags are cached per bounded facade and refreshed when class layout flags change; no separate unbounded layout cache is introduced. Metadata as a whole remains process-lifetime storage, not an unloadable global memory budget.

Reproduce after configuring src/tests/net10-contracts (or its Emscripten target):

```powershell
python scripts/dotnet10/resource-budget.py --runner out/net10-contracts/Release/net10-resources.exe --baseline out/net10-resource-baseline.exe --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8" --output out/net10-resource-comparison.json
python scripts/dotnet10/resource-budget.py --runner out/net10-contracts-wasm/net10-resources.js --node --runtime out/net10-aot-current/wasm/bcl --output out/net10-resource-wasm.json
```

The optional baseline executable is a local artifact; omit --baseline for a fresh checkout, or retain an executable from a previous revision before rebuilding. Raw five-run samples are stored in the JSON outputs, under locally excluded out/. Linux/ARM64 and UBSan remain external acceptance gates until their CI runs are available; this document does not count Windows or Wasm as those results.
