# Android report follow-up and public ABI repairs

Date: 2026-11-13

## Scope

The third-party Android/UE report was checked against the current runtime.
UE packaging, Cook settings, and host repository changes are outside this change.
The shared runtime fixes below are applied to `mono-minimal`, `mono-full`, and
`coreclr-minimal`. Adapter-specific fixes affect `coreclr-minimal` only.

## CoreCLR adapter fixes

| Issue | Final behavior | Commits |
| --- | --- | --- |
| POSIX error conversion used an obsolete helper | Calls the existing `errno_to_io_error`; native Linux builds succeed | `537b3c4f` |
| Non-Windows NLS placeholders returned misleading results | Minimal hosts select CoreLib invariant algorithms; unavailable native NLS entrypoints report NotSupported; Windows keeps native NLS | `3088131d` |
| POSIX environment-variable lookup was a stub | Checked UTF-16/UTF-8 conversion, Unicode values, correct capacity/terminator/error semantics, fallible temporary buffers | `71ad4bde` |
| Property attribute-provider overloads had incompatible signatures | Exact two-parameter property bindings support IsDefined and attribute construction | `55b8e106`, `f47a3096` |
| Platform coverage did not enforce genuine AOT entry execution | Dedicated interpreter/AOT platform gates, environment contracts, stock .NET comparisons, compatible CoreLib provenance | `05537855`, `6931c8f9`, `809375c1` |

Property adaptation deliberately preserves CoreLib's existing field and other
provider implementations. The regression gate checks FieldOffset pseudo-attributes
as well as property metadata, constructor side effects, missing attributes, and
null attribute types. The shared reflection fixtures were also run against both
Mono branches.

See [minimal globalization](minimal-globalization.md) for the policy. The validated
CoreLib is Windows-flavor .NET 10.0.8 with matching pointer width: win-x64 on native
64-bit hosts and win-x86 on Wasm32. This is not a claim of compatibility with
arbitrary Unix/Mono runtime packs or full non-Windows linguistic globalization.

## Public invoker ABI fixes

All five registration APIs now use typed adapters: PInvoke, internal call,
new-object internal call, intrinsic, and new-object intrinsic. No five-parameter
void callback is called through a four-parameter RtResultVoid function pointer.
The adapter supplies a real, initially null exception output, transfers managed
exceptions to the runtime, and returns the internal success/error protocol.
The public C callback signature remains unchanged.

Callback bindings are indexed by actual method identity and registration kind.
Warmed calls use an O(1) hash lookup without allocating or formatting names.
Name ownership is retained by the bridge, and binding-cache allocation failure
leaves existing bindings valid and permits retry. Registration and binding follow
the runtime's existing owner-thread and metadata-lifetime assumptions.

`leanclr_invoke_with_buffer` now clears `*out_exception` on success after checking
for null. The same stale-output pattern was fixed in assembly loading and
assembly enumeration. Invalid method/argument/return-buffer combinations return
managed errors without reaching invalid invocations.

| Repair | coreclr-minimal | mono-full | mono-minimal |
| --- | --- | --- | --- |
| Typed C callback adapters | `87753bd2` | `12453271` | `cdec2e8e` |
| Exception-output clearing and buffer checks | `434b208f` | `2212eb52` | `be8fa888` |
| Scan both 32-bit pointers in an eight-byte stack slot | `f26e3528` | `7c10361e` | `9910c179` |
| Exclude numeric local storage from conservative root scanning | `ac04ae6a` | `ccff35ee` | `6f95ddd0` |

The ABI follow-up found another pointer-width defect: an unboxed two-reference
structure fits in one eight-byte interpreter slot on 32-bit targets. Scanning
only its first pointer loses the second live object. Every pointer word is now
scanned, with a regression reproduced on Win32 before the fix.

Linux regression testing additionally found stale object addresses in numeric
locals of SkipLocalsInit methods. These falsely retained finalizable objects.
The transformer records coalesced numeric-local exclusion ranges; references,
structs, native integers, pointers, and live evaluation storage remain scanned.
Each interpreter method adds one pointer; only methods with numeric locals allocate
range storage (four bytes per coalesced range plus a four-byte terminator).
GC scanning adds no allocation and avoids scanning those known scalar locals.
The fix and a deterministic native regression are shared by all three branches.

## Verification

- CoreCLR Windows x64, Wasm32, and native Linux x64: platform contracts pass in
  both interpreter and real AOT execution. The AOT runner rejects an interpreted
  entrypoint, while normal interpreter fallback within callees remains allowed.
- Existing CoreCLR smoke, GC, and API suites: **98/98 pass on each of Windows,
  Wasm, and Linux** after the final fixes.
- Real C-compiled callbacks cover all five registration APIs, success and managed
  exception handoff, temporary name lifetime, allocation failure/retry, and
  10,000 warmed calls without additional general allocations.
- C ABI and deterministic GC-root contracts pass on CoreCLR Windows/Linux/Wasm,
  both Mono native x64 builds, and Mono minimal Win32.
- Environment contracts cover Unicode, long values, missing variables, capacity
  queries, insufficient buffers, and invalid inputs. Stock .NET comparisons pass.
- Property attribute and field pseudo-attribute regressions pass. Shared C++ files
  were formatted with the repository clang-format; git diff checks pass.

The platform workflow now runs the gates and uploads their logs and CoreLib
provenance. Local results live under `out/platform-contracts` and
`out/android-report-audit`; these generated artifacts are not committed.
Android NDK/device execution and integration with UE packaging were not exercised.
Remote Actions results must be assessed separately after pushing.
