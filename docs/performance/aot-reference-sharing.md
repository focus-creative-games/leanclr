# Bounded reference-generic AOT code sharing

`--leanaot-share-reference-generics` opts in to canonical code for closed generic
instances whose generic arguments are all managed references. The default is
off. Actual registered identities and their real `RtMethodInfo` contexts remain
unchanged; only the generated body is canonicalized to Object arguments.

## Conservative boundary

Eligible bodies only move reference/scalar arguments and locals, branch and
compute without type-dependent metadata. Value-type declaring classes, value
arguments, struct/byref/pointer signatures, EH, address-taking/native-control
operations, every token-bearing operation and array manipulation stay
specialized. In particular typeof, newobj, generic static/instance fields,
boxing, casts and constrained calls do not share yet. Array references may be
passed/returned without sharing array element operations. Unsupported sharing
shapes still get ordinary specialized AOT when eligible, not blanket
interpreter fallback.

This deliberately avoids sharing per-specialization static metadata caches.
Broadening eligibility requires a context-keyed metadata design and separate
validation; matching native signatures alone is not sufficient evidence.
Eligibility scans each definition's IL once, then checks instance signatures.

Definitions/declarations/direct-call casts use the canonical signature. The
caller still resolves and passes its actual closed method. Dynamic/reflection
invokers preserve exact actual stack offsets and adapt the common reference
ABI. The registry keeps every structural identity and hash collision check;
there is no runtime canonical-method lookup, new heap map, per-call wrapper or
new mutable cache. Unseen reflection instances remain interpreted even if a
compatible canonical body exists. Instance/depth budgets continue counting
actual registered instances.

## Validation and reproduction

Six new formal cases cover cross-assembly reference and array arguments,
class plus method arguments, shared arithmetic errors, reflection/unregistered
instances, distinct generic statics and type-dependent specializations, loops
and null. Native checks independently require identical code pointers in shared
mode, distinct runtime methods/contexts, the actual string argument class and
interpreter fallback for an unregistered reference. Existing wide-return,
generic EH, interface and registry collision tests run alongside these cases.

```powershell
python scripts/test/reference-sharing.py --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_reference_sharing.py --build-only --emsdk D:/workspace/wasmclr/emsdk
# After builds finish:
python scripts/benchmarks/compare_reference_sharing.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

The matrix covers combined return protocols on Native/Wasm, a specialized
control on Native/Wasm, default Result Debug and Win32, all with generics
on/off. CI also combines sharing with the two-entry interface cache and all
eight return flag combinations.

The isolated benchmark registers the same four string/object/class/array
instances in both modes. It verifies four specialized bodies versus one
canonical body with sharing, preserves identical managed bytes, noinline
leaves and compiler settings, and measures each workload separately. Runtime
speed is not expected to improve simply by deduplicating code. Seven randomized
interleaved process samples run after builds; browser/game, instruction-cache
pressure and metadata-dependent sharing are outside this measurement.

## Recorded result and decision

All twelve formal configurations passed 71 managed cases each, including the
six sharing cases (72 executions), registry checks and mixed return targets.
The formal fixture keeps 129 registered identities while emitting 119 bodies
in sharing mode. The isolated benchmark reduces four bodies to one and generic
body C++ source from 13,340 to 3,329 bytes (about 75%).

Final native artifact size remains 870,912 bytes. Wasm changes 825,060 ->
825,054 bytes, with code section 659,985 -> 659,982 bytes and gzip +4 bytes.
These final artifact differences are negligible. Loop timing shows no
consistent improvement; compiler loop transformations also mean that these
numbers are not isolated generic-call costs.

**Keep sharing opt-in.** This establishes safe canonical code generation and
reduces duplicated source/compilation work, but this reference-only,
metadata-independent subset does not demonstrate material Wasm binary savings
on this toolchain. Do not extrapolate source reduction into binary reduction,
or broaden sharing to metadata-dependent methods without separate context
storage and validation. Raw timings, source/artifact hashes, build manifests
and validation reports are in
[aot-reference-sharing.json](data/aot-reference-sharing.json).
