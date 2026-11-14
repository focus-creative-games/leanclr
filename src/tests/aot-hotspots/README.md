# Production AOT hotspot boundaries

```powershell
python scripts/test/aot-hotspots.py --all-protocols
python scripts/test/aot-hotspots.py --gc-debug
```

The same optimized managed fixture runs on .NET 10, the LeanCLR interpreter,
default AOT, combined lightweight/packed32/status-out return ABIs, and reference
generic sharing. The native host requires the positive Casts/Delegates/Arrays
entries to have AOT invokers. Exception-catching drivers may use the existing
EH fallback; their cast/call/newarr children still have generated bodies.

Coverage includes null and invalid casts, inheritance, sealed classes,
interfaces/variance, array covariance, generic casts and nullable boxing;
static/closed/open/virtual/boxed/generic delegates, null-bound static targets,
multicast ordering and exceptions, reentrancy, reference/enum/void/byref/wide/
structure returns and an explicitly interpreted target; delayed and failed
class initialization; zero/negative/variable array lengths, primitive/reference/
reference-containing struct arrays, exact generic array runtime types, zeroing,
injected allocation failure, accounting and recovery.

Native bindings also exercise 32/64-bit enum/underlying return aliases in both
directions, including packed/status-out ABI fallback. Mono BCL's reflection
CreateDelegate validator still rejects these aliases; this test constructs the
runtime binding directly and does not claim reflection API compatibility.

The host also repeatedly collects between managed calls. A static rooted
reference-containing struct array and its nested object must survive, and
releasing the root must reach stable retained object bytes. Collection remains
at host boundaries; these tests do not enable automatic GC during managed code.
Retained object accounting is not process RSS or reserved arena capacity.

The allocation cache is limited to two pointer widths. No delegate or array
instance layout changes are permitted by these optimizations.

Allocator boundaries additionally cover 16/24/96/256/8192-byte blocks, the
maximum 2048-slot arena, word boundaries, unused trailing bytes, partial sweep
followed by recycled/virgin allocation, full exhaustion and refill. Survivor
payloads stay intact, blocks are zeroed, debug quarantine prevents reuse, and
the arena metadata size budget is unchanged. Fault injection covers capacity
reservation and arena metadata failure after acquiring aligned payload, then
recovery and balanced release. Reference-array elements still finalize exactly
once after their array root is released; debug array allocations preserve sites.
