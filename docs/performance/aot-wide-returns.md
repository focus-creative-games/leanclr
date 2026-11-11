# AOT wide and struct status/out trial

`--leanaot-status-out-returns` is an independent opt-in for IL long, ulong,
double and non-enum value-type returns. The generated function physically
returns `RtErr` and appends an explicit, caller-owned output pointer after the
actual generic context argument. Success is `RtErr::None`; only success writes
the output. No payload bit pattern is reserved. There is no heap allocation,
thread-local error slot, or shared result storage.

Enums (including long-backed enums), float, small integers, byrefs and native
pointers keep their previous protocols. Void/reference and packed int/uint
remain separate switches. The default remains Result throughout. Runtime API,
icall, pinvoke and public invocation interfaces retain their checked Result
contract; special entry wrappers adapt those results at the AOT boundary.

Ordinary/generic definitions, direct calls, virtual adjustment thunks and
invokers use the same eligibility policy. Logical adapters at call sites are
inline candidates, not assumed free. Dynamic bridges inspect the actual target
flags and actual generic-context ABI, supporting legacy and trial targets in
either caller mode. Enum eligibility and physical invoker ABI participate in
deduplication keys. Bit 2 enables status/out; bits 0/1 retain their meanings.
Registration accepts flags 0 through 7 and rejects unknown bits before mutation.
No metadata structure gains a field.

Regenerate and rebuild **all** AOT callers, targets and the runtime together.
Existing generated callers cannot discover the new ABI merely because layout
is unchanged. The public C ABI remains unchanged.

## Validation

Seven formal managed cases cover long/ulong extremes, NaN/infinity/negative
zero, structures containing references, cross-assembly closed generics,
interfaces (including boxed value-type receivers), long enums, divide/overflow,
reflection/delegates and generic EH interpreter handoff. Native host injection
checks all eight target flags, success payloads and OOM propagation through an
actual generated interface bridge. Standalone tests verify all 255 nonzero
error-byte encodings and that failure leaves output untouched.

```powershell
python scripts/test/wide-returns.py --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_wide_returns.py --build-only --emsdk D:/workspace/wasmclr/emsdk
# After all builds finish:
python scripts/benchmarks/compare_wide_returns.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

The runner covers legacy and combined trial Native/Wasm configurations,
independent status/out Win32 and combined Debug. Generic enabled/disabled
variants provide a behavioral oracle. The standalone suite also emits LLVM
IR and asserts a scalar status return without `sret` for the exported wide
probe. Logs are under `out/wide-return-validation`.

## Measurement boundaries

Separate noinline long, double and small-struct leaves run in otherwise identical
loops. Both builds use byte-identical managed inputs and matched toolchain
settings. Seven randomized, interleaved process samples run after builds.
This measures the implemented entry ABI plus inline logical adapters, not an
abstract status/out ideal. Artifact sizes exclude external BCL/IL/resources.
Large structures, exception-heavy workloads, browser/game integration and
IL2CPP comparisons require their own measurements. A successful ABI probe does
not establish a performance win on every native ABI.

## Recorded results and decision

Median ns per iteration, seven samples of 5,000,000 calls:

| Return | Native Result | Native status/out | Wasm Result | Wasm status/out |
| --- | ---: | ---: | ---: | ---: |
| long | 0.793 | 0.792 | 1.829 | 1.822 |
| double | 2.377 | 2.414 | 2.463 | 2.446 |
| small struct | 4.750 | 4.495 | 7.548 | 7.547 |

Long and double differences overlap the sample ranges; no reliable improvement
is established. The small struct is about 5.4% faster natively, with disjoint
sample ranges, but essentially unchanged in Node/Wasm. Native size stays
869,376 bytes. Wasm decreases 823,954 -> 823,885 bytes (code section -73 bytes),
while gzip increases 22 bytes. These are tiny size differences.

**Keep this trial off by default.** It provides a genuine explicit-out ABI and
complete boundary adaptation, but this measurement does not justify enabling
it uniformly for wide scalars and structures, particularly for Wasm. Native
applications may opt in after measuring their own structure sizes and workload.
The scalar status alone does not remove payload stores or caller-side copies.

Validation passed twelve formal configurations (65 managed cases each), each
also injecting eight target flag combinations, plus three standalone builds
and the Wasm scalar-return IR assertion. Raw samples, artifact hashes,
generation inputs and validation reports are in
[aot-wide-returns.json](data/aot-wide-returns.json).
