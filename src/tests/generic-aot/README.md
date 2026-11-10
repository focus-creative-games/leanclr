# Closed generic AOT tests

Use `python scripts/test/aot-features.py --target native` (or `--target all
--emsdk ...`) from the root. The runner generates ordinary and zero-instance
configurations and verifies both, rather than inferring AOT coverage from a
successful managed result.

Coverage includes value/reference/array/nested generic arguments, generic class
constructors and static constructors, methods on generic value types, boxing,
constraints, recursion, methods with identical native signatures but different
generic arguments, cross-assembly calls and native reflective invocation.
Unregistered instances must stay interpreted instead of borrowing another
specialization. Generic exceptions still propagate through the existing Result
protocol.

The planner test program additionally checks deduplication, instance/depth
budgets, attribute exclusion, unselected assemblies and conservative EH fallback.
The closed-instance budget is global to one generation run. Discovery follows
statically referenced methods from selected AOT roots; it is not a guarantee to
compile arbitrary future reflection targets or all virtual overrides.

CLI controls:

- `--leanaot-max-generic-instances 2048` (default); use `0` to disable discovery.
- `--leanaot-max-generic-depth 32` (default).

`generic-aot-report.json` lists instances, generic fallback edges, ordinary EH
fallbacks and normal-path EH entries. Unsupported argument forms (e.g. function
pointers or shaped arrays) retain interpreter fallback.
Methods using managed `calli` also stay interpreted: its signature-only direct
call path cannot safely carry the closed-instance context. Managed delegates
use runtime invokers and are covered by the fixtures.

Rebuild the runtime and regenerate **all** AOT sources together. The internal
module registration table gained closed-instance identities; closed generic AOT
functions also carry their runtime method context. Ordinary method signatures
and the public C API/Result protocol are unchanged. Old prebuilt registration
tables are not binary-compatible with this internal table extension.

The default AOT return signatures remain unchanged. The separate opt-in
`--leanaot-lightweight-returns` generator flag changes internal void/managed
reference return signatures and records the protocol in module registration.
Use `scripts/test/aot-features.py --lightweight-returns ...` to verify that mode.
Both modes exercise the standalone AOT result tests and interface bridges into
targets using each return ABI, including null success and OOM propagation.
See [the return protocol report](../../../docs/performance/aot-return-protocol.md).
