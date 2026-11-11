# Closed generic EH normal-path AOT

Closed generic methods with catch/finally can now compile their normal paths.
An exception transfers the current arguments, locals and IL offset to the
interpreter, which continues the same invocation. It does not restart the
method, replay prior effects, or return to AOT after the catch.

## Eligibility and context

The planner inflates parameters, return types and locals before checking the
snapshot contract. Primitive values and managed references (including arrays
and constructed reference types) qualify. A struct generic argument is allowed
when the actual snapshot only contains references/scalars. Open definitions,
value-type `this`, struct/enum/byref/pointer snapshots, address-taking/native
control operations, uninitialized locals, filters/faults, EH inside finally,
and snapshots over 128 argument/local values retain interpreter fallback.
Existing instance/depth budgets and explicit exclusions still apply.

Error handling and handoff use the actual closed `__aot_method`. Resolving the
MethodDef token would lose its class/method generic context. Reflection
intrinsics that intentionally expose the definition retain their old behavior.
No runtime metadata layout, return ABI, global registry lookup, or public API
changes are required by this step. The report adds `generic_normal_path_methods`.

Snapshots use fixed stack storage bounded by the eligibility policy. Normal
calls do not generate interpreter IR; the first exception initializes it lazily
and subsequent exceptions reuse it. Existing frame savepoints restore the
interpreter stack after both handled and escaping exceptions. This does not
introduce a new AOT GC-root mechanism; automatic-GC pressure/liveness is outside
this validation's existing runtime contract.

## Validation

Thirteen formal managed cases cover signed/unsigned/floating boundaries, NaN,
infinity, null, foreign-assembly references and structs, mutated arguments,
closed catch types, nested finally, throwing finally, rethrow, constructors,
class plus method contexts, void/array returns, post-catch continuation and
conservative fallbacks. Native host checks independently assert actual AOT
binding, lazy IR creation/reuse, wrong-snapshot rejection and frame cleanup.

Reproduce the archived matrix with:

```powershell
python scripts/test/generic-eh.py --emsdk D:/workspace/wasmclr/emsdk
```

It covers all four return-protocol combinations, Release x64 and Wasm, Debug
x64 and Release Win32, with generic AOT enabled/disabled. Logs and reports are
under `out/generic-eh-validation`. Generic-disabled executions are a separate
behavioral oracle rather than a claim of AOT coverage.

## Isolated benchmark

```powershell
python scripts/benchmarks/compare_generic_eh.py --build-only --emsdk D:/workspace/wasmclr/emsdk
# After all builds finish:
python scripts/benchmarks/compare_generic_eh.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

Both modes use identical managed DLL bytes, AOT wrappers and compiler options.
Only the two noinline `GenericEhLoop<int/string>` instances change backend.
Other return and interface trials remain disabled. Each success-path workload
checks its finally count and checksum; the host verifies that AOT creates no
interpreter IR. Seven interleaved process samples run after builds. These
measure native optimization versus interpretation of a loop, not isolated
call overhead, exception-heavy performance, first-exception latency or IL2CPP
parity. Artifact sizes exclude external BCL/IL/resources.

## Results on the recorded Windows/Node host

The full matrix passed 96 configurations (64 feature, 32 formal); the formal
suite ran 58 cases in each configuration, including 416 generic EH case
executions. The independent runtime regression suite passed 41/41 tests.

Median ns per loop iteration, seven samples of 5,000,000 iterations:

| Workload | Native interpreted | Native AOT | Wasm interpreted | Wasm AOT |
| --- | ---: | ---: | ---: | ---: |
| Generic EH int | 18.045 | 1.289 | 37.767 | 0.842 |
| Generic EH string | 18.292 | 1.305 | 19.068 | 0.826 |

These loops improve about 14x natively and 23–45x in Node/Wasm. Native artifact
size grows 869,888 -> 871,936 bytes; Wasm grows 824,450 -> 826,019 bytes, with
code section 659,692 -> 661,103 bytes. This is the cost of adding two native
bodies, not a claim that normal-path EH reduces size versus interpretation.
Wasm gzip size falls 796 bytes; compression is not an instruction-size metric.

Raw samples, toolchains, hashes, artifact sizes and validation records:
[aot-generic-eh.json](data/aot-generic-eh.json). Timing varies with host load;
the bounded functional contract is the primary acceptance criterion.
