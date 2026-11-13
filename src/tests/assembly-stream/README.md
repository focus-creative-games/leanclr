# Default-context assembly stream loading

This profile supports `AssemblyLoadContext.Default.LoadFromStream` with copied,
process-lifetime PE and portable-PDB data. It preserves CoreLib stream reading:
positioned memory streams, exposed buffers, short reads, closed streams and read
errors. A stream whose Length/Position are unsupported is rejected by CoreLib,
matching .NET 10; this does not claim general non-seekable-stream buffering.

Dependencies resolve on demand. The same assembly identity and MVID reuse the
existing RuntimeAssembly even if non-semantic PE padding differs; a conflicting
definition raises FileLoadException. Missing dependencies raise FileNotFoundException
and can be supplied by a later stream load. Invalid PDB data is ignored as in the
reference runtime; its native buffer is released.

Custom and collectible contexts are rejected. The native initialization boundary
validates GC handles, retains only the default strong handle, and releases handles
and suppresses finalization for rejected constructors. The opaque default token
is never dereferenced as a managed object or an OS handle.

The native file-stream regression also covers a legacy binding error: CoreLib's
CreateFilePrivate returns SafeFileHandle, so the Mono IntPtr interception must not
replace its managed wrapper. The raw P/Invoke still returns an OS handle, which
CoreLib wraps and disposes normally.

## Build and test

```powershell
./src/tests/assembly-stream/build-fixtures.ps1
cmake -S src/tests/assembly-stream -B out/assembly-stream/native -A x64 `
  -DLEANCLR_BCL_ADAPTER=CORECLR_NET10 `
  "-DLEANCLR_STREAM_MANAGED_DIR=<absolute out/assembly-stream/managed>" `
  "-DLEANCLR_STREAM_RUNTIME_DIR=<matching .NET 10 CoreLib directory>"
cmake --build out/assembly-stream/native --config Release --parallel 4
ctest --test-dir out/assembly-stream/native -C Release --output-on-failure
```

The fixture DLLs are embedded as generated Base64 inputs, never referenced by the
managed driver, so tests genuinely load their bytes. `Other` has the same assembly
name but a different definition. Native Windows adds real FileStream cases. Wasm
uses the Emscripten toolchain, portable host and a **32-bit** CoreLib, with fixture
and BCL directories beneath the test-only `/host` Node filesystem mount.

The native ownership test verifies valid PDB retention, repeated duplicate and
malformed loads, injected allocation failures (or successful pool retry), rejection
of invalid context tokens, caller ownership of unrelated handles, and stable
strong-handle counts after repeated rejected-context construction and GC.

Shared loader changes retain the original two-argument ABI and eager-reference
behavior for existing callers. Stream loading uses the explicit deferred-reference
overload. Module display names belong to their metadata pool, and ignored PDB
buffers no longer escape failed/ignored-symbol cleanup.
