# Global generic AOT registry

This round replaces definition-assembly generic lists with one immutable index
per LeanAOT generation. It addresses first-binding cost and cross-assembly
ownership; it does not change the default Result ABI or claim IL2CPP parity.

## Ownership and discovery

`generic_methods_part*.cpp` contains closed-instance bodies, split using the
existing code-size limit. `generic_registration.cpp` contains their identities,
invokers, function pointers and return protocols. Ordinary selected assemblies
retain their existing registration files. Generic argument descriptor sequences
are interned across the whole generation, including nested sequences.

`-a` selects ordinary AOT roots. A reachable closed generic can be generated
even when its definition assembly has no ordinary AOT registration. Its class
and method arguments can come from other loaded assemblies. Explicit attribute
and XML exclusions still apply. IL metadata tokens inside a generic body are
resolved against the runtime method's **definition image**; global storage does
not change token scope.

Discovery now also follows static-field-only generic class initialization.
Constructing a closed generic class discovers its concrete nonabstract virtual
methods that have no additional method generic parameters. Arbitrary future
reflection instances and unanchored generic virtual overrides remain interpreted.
Instance/depth budgets, unsupported argument forms and existing EH/calli
fallback rules remain in effect. This round preserves the previous normal-path
EH implementation and interpreter handoff.

## Lookup and memory

The generator sorts entries by a 64-bit FNV-1a hash of the definition assembly,
MethodDef token, and structural class/method argument identities. Names are
UTF-8 and integer encoding is explicitly little endian. Runtime binding hashes
the requested context, binary-searches the array, then compares complete
identities within the equal-hash range. An argument identity includes its
defining assembly, so matching type names or native signatures are insufficient.
Hash collisions cannot bind a different specialization.

Lookup costs O(argument shape + log N + collision checks), replacing an O(N)
scan. `Shim::setup_method` performs one binding and assigns invoker, direct and
virtual pointers together. The existing pooled generic-method cache retains
both AOT and interpreter results. Invocation does not search this registry.
The selected return protocol is cached in existing `RtMethodInfo` padding;
layout assertions cover x64, x86/Wasm and the PGO field configuration.

The generic index uses generated static storage and adds no runtime heap map or
eager argument-assembly loading. Normal module, type and method caches still
allocate. Each registry entry grows by eight bytes (64 to 72 on x64; 32 to 40
on x86/Wasm). Global descriptor interning offsets that cost for this fixture;
the actual balance depends on the instance set. Registration checks table
ordering, required storage, protocols, tokens and function pointers before
replacing the index. This is validation of trusted generated tables, not a
general validator for arbitrary native pointers.

**Rebuild the runtime and hosts and regenerate all AOT sources together.**
Internal module/global registration layouts changed and old generated tables
are incompatible. Public C API signatures remain unchanged.

## Validation

`scripts/test/global-generics.py` builds the real managed `AotTests` project and
its Common and GenericAotArguments dependencies. Twenty-four formal test methods
cover primitive/reference/value/nested types, three-assembly contexts, same-name
types from distinct assemblies, generic statics and constructors, constraints,
interfaces, virtual methods, delegates, reflection hit/miss, recursion, arrays,
nullable values, boxing, byref returns, exceptions and explicit fallback.
Native-side checks additionally verify actual AOT/interpreter selection,
definition-assembly independence, descriptor depth boundary, malformed tables,
forced collisions, failed-registration state preservation and method layout.

All 16 configurations passed: default/disabled generic discovery, Result/
lightweight return protocols, and x64 Release, x64 Debug, Win32 Release or Wasm
Release. These execute 384 formal test methods in total. The previous generic/EH
feature matrix also passed all 32 configurations (48 CTest executions).
Runtime regressions passed 41/41 and GC regressions 166/166. The full managed
run passed 5175/5176; the existing
`TC_System_Console::TreatControlCAsInput_ToggleWithoutThrow` failure remains.
CI now includes the formal suite in its AOT return-protocol matrix. Local
validation was on Windows and Node/Wasm; Linux/ARM execution was not performed.

## Reproduction and measurement limits

```powershell
python scripts/test/global-generics.py --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/global-generics.py --config Debug
python scripts/test/global-generics.py --arch Win32
# Repeat with --lightweight-returns for the other return protocol.
python scripts/benchmarks/run_generic_lookup.py --wasm --samples 7 --iterations 20000
```

The benchmark constructs 64/1,024/16,384-entry tables, measures a middle hit and
a miss, and compares sorted-hash lookup with a structural linear scan using
the **same new entry layout**. Metadata is already loaded. This isolates the
lookup algorithm; it is not a historical binary comparison, startup benchmark,
or end-to-end generic-call benchmark. Synthetic table allocations belong to
the test harness. Measurements include seven process samples, raw rows,
medians/ranges, source and artifact hashes in the accompanying JSON. Nominal
metadata comparisons use the same real fixture instance set and exclude code,
strings, relocations, module containers and linker folding. Prior performance
snapshots are preserved.

Seven-sample medians in nanoseconds (linear / sorted hash):

| Entries | x64 hit | x64 miss | Wasm hit | Wasm miss |
| ---: | ---: | ---: | ---: | ---: |
| 64 | 25.87 / 16.63 | 42.66 / 18.12 | 51.87 / 46.78 | 57.41 / 54.07 |
| 1,024 | 180.18 / 19.17 | 452.29 / 19.44 | 221.23 / 46.42 | 514.22 / 45.57 |
| 16,384 | 3,553.38 / 21.87 | 7,222.60 / 21.87 | 4,984.07 / 66.82 | 10,227.32 / 61.77 |

The small Wasm table has overlapping sample ranges; the clear benefit is
scaling to larger tables. The fixture produces 68 instances and 61 interned
type descriptors instead of 228 repeated descriptors. Nominal entry-plus-type
storage changes from 13,472 to 7,336 bytes on x64 and 6,736 to 3,940 bytes on
Wasm. These are computed storage comparisons, not whole-binary size reductions.
Raw measurements and provenance are in
[global-generic-registry.json](data/global-generic-registry.json).
