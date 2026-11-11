# .NET 10 AOT execution contract

## Validated behavior

Windows x64, Microsoft.NETCore.App 10.0.8:

- 77 smoke, 15 real-heap GC/EH, and 5 positive/negative API entries pass.
- The same 97 entries pass with lightweight reference/void returns, packed int32
  returns, wide/struct status-out returns, and reference generic sharing enabled together.
- All 97 default-protocol entries pass under MSVC AddressSanitizer.
- Closed generic, disabled-generic comparison, mixed return ABI and normal-path EH
  harnesses pass for .NET 10. The Mono versions retain the same harnesses.

Reproduce:

```powershell
python scripts/dotnet10/runtime-bindings.py --check
python scripts/dotnet10/aot-contracts.py --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8"
python scripts/dotnet10/aot-contracts.py --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8" --variant optimized
python scripts/test/aot-features.py --target native --profile coreclr-net10 --runtime-dir "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8"
python scripts/test/aot-features.py --target native
```

For ASan, add the MSVC ASan DLL directory to PATH and use
`aot-contracts.py --configuration RelWithDebInfo --asan`. Results and per-entry logs
are written below `out/net10-aot-current`; this directory is not committed.

## ABI, binding and root policy

`ldtoken` and typed references use canonical MethodTable facades in this profile.
Inline arrays of length one are valid. Generic `sizeof(T)` uses the instantiated
storage type, including pointer size for reference instantiations.

`runtime-implemented.json` is a reproducible snapshot of adapter binding names.
The planner records `runtime-native-binding` and never compiles the self-recursive
IL placeholders behind those bindings. Unknown intrinsic bodies stay on the runtime
path with a separate reason. Static interface implementations resolve once during
metadata initialization, retaining the actual generic context and caching errors.

Collections are synchronous in this single-threaded profile. AOT frames expose
stable parameter/local ranges and copy live evaluation values at calls and static
field accesses that can run managed code. Byref targets are revisited at collection,
so assignments through a byref remain visible. The snapshot capacity is fixed at
compile time; leaf methods have no root scope and scalar-only call scopes disappear
under optimization. This avoids scanning native stacks or reserved maximum stacks.
The Mono profile does not generate these .NET 10 root scopes.

## Exception boundary

Normal paths and supported normal finally paths execute as AOT. At the first throw,
the interpreter resumes exception dispatch from the faulting IL with the current
argument/local snapshot. Normal-path side effects are not replayed. Catch-only
address operations do not disqualify that snapshot; aliases created on the normal
path, filters, fault handlers, and unsupported finally shapes retain explicit whole
method fallbacks. The harness verifies counters, mutated arguments, rethrow,
finally ordering, and frame balance against its interpreted comparison.

## Varargs boundary

Managed varargs and unmanaged/native calli require separate argument descriptors,
ABI-specific lowering, root descriptions, and exception snapshots. They must not be
routed through an ordinary fixed-argument bridge. This migration preserves explicit
planner/runtime rejection rather than guessing an ABI; complete ArgIterator/native
varargs execution is outside the minimal profile. Platform-specific calli work must
provide separate Windows, SysV, ARM64 and Wasm contracts before enabling it.

## Remaining platform gate

The first Wasm build exposes unguarded Windows QCall bindings and pointer-size facade
assumptions in the imported adapter. Those are tracked by the platform stage, not
hidden by native passes. Linux/ARM64 and UBSan have not passed locally: WSL's
registered ext4.vhdx is missing.
