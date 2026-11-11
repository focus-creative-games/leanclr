# Core memory ownership and failure policy

## Host callbacks

`GeneralAllocation::set_memory_callbacks` returns `bool`. Configure all seven
callbacks on the single host/runtime thread **before the first allocation
attempt**, including before `Settings::set_command_line_arguments`, file loading,
GC setup, or `Runtime::initialize`. An incomplete table returns `false` without
changing the current table. Once an allocation has been attempted, changing any
function pointer returns `false`; installing the identical table returns `true`.
There is no reset/unlock operation, including after individual buffers are freed.

```cpp
#include "alloc/general_allocation.h"

leanclr::alloc::MemoryCallbacks callbacks = /* seven compatible callbacks */;
if (!leanclr::alloc::GeneralAllocation::set_memory_callbacks(callbacks))
{
    // Report a host configuration error before starting the runtime.
    return false;
}
```

Callbacks must remain callable for the lifetime of all runtime storage, including
static destruction. `malloc`/`calloc`/`realloc` storage must be accepted by `free`;
aligned storage must be accepted by `aligned_free`/`aligned_realloc`. Failed
nonzero reallocations preserve the old allocation. Callbacks follow their C
allocation counterparts for zero-sized requests. Aligned requests require a
nonzero power-of-two alignment. Do not change the underlying allocator identity
through mutable callback state while allocations are live. Failure injection can
change whether allocation succeeds while retaining the same ownership backend.
Callbacks must not call `GeneralAllocation` recursively.

## Coverage

The callback table owns managed heap arenas, `MemPool` regions and their region
records, the global metadata pool object, `utils::Vector` buffers, invocation
scratch buffers, raw GC handle records, GC size-class descriptors and debug
allocation-site records. These objects use matched `GeneralAllocation` allocation
and release operations. Placement new only constructs objects in storage already
owned by its enclosing pool or allocator.

The callback table does **not** control every byte in the process. In particular,
`utils::HashMap`/`HashSet` currently alias standard containers with their default
allocator: metadata caches, module maps, the GC handle index and GC tracking sets
still allocate through the C++ library. Standard strings/streams, profiling
containers, platform-owned resources and compression/zlib storage also use their
own allocators. The handle record and its index therefore have different
allocation domains. Migrating these containers requires a separate fallible or
explicitly fatal container contract; the nullable `GeneralAllocator` used by
`utils::Vector` must not be installed as a standard-container allocator.

Loader inputs with `shared == false` transfer ownership to `Assembly::load_from_data`
on both success and failure and must be releasable by the installed `free`
callback. Inputs with `shared == true` remain host-owned and must stay valid while
the loaded module is in use. Module/metadata storage generally lives for the
runtime lifetime; these changes do not add a full unload/reset facility.

## Shared primitives

- `utils::CheckedSize`: checked add, multiply, alignment and bounded ranges.
  Failed arithmetic leaves its output unchanged. It returns a boolean so each
  boundary can select its appropriate `RtErr`.
- `utils::BinaryReader`: bounded little-endian parsing. `try_read_slice` gives a
  child only its declared payload; failed slicing preserves both readers.
- `alloc::ScopedBuffer<T, N>`: zeroed POD scratch memory, inline up to `N`
  elements and callback-owned above that. Replacement/overflow failure preserves
  existing storage and contents. The buffer cannot be copied or moved.
- `utils::ScopeExit`: one typed cleanup, installed without a heap allocation.
  Cleanup is nonthrowing, runs once in reverse declaration order, and can be
  dismissed after ownership transfers.
- `vm::ArrayLayout`: one checked physical layout for array construction, cloning
  and GC size accounting. Empty arrays retain their full multidimensional bounds;
  bounds remain aligned to eight bytes.

## Failure boundaries

Keep `RtResult` at runtime boundaries; do not throw a C++ exception for a failure
that already has an error return channel. The default runtime build disables C++
exceptions. Existing `assert` checks describe internal preconditions, not OOM
handling.

| API/domain | Failure behavior |
| --- | --- |
| `CheckedSize`, reader `try_*` | `false`, with documented output/position preservation; parsers translate malformed input to `BadImageFormat` |
| `GeneralAllocation`, `MemPool`, `ScopedBuffer::try_allocate_zeroed`, `Vector::try_*` | `nullptr`/`false`; callers check before dereference |
| Managed object/array/string/handle allocation and invocation scratch preparation | Translate allocation failure to `RtErr::OutOfMemory`; negative dimensions use `Overflow` |
| Array physical size arithmetic | Fallible `Array::get_array_layout` returns `OutOfMemory` on size overflow; existing-object size getters use fatal policy if their invariant is broken |
| Assembly loading before metadata publication | Checked pool/image/PDB/assembly/module allocation failures return `OutOfMemory`; scoped rollback releases acquired storage |
| Global `MetadataAllocation`, `Vector::reserve`/infallible growth, GC descriptor/site initialization | No recoverable return channel: `panic`/abort on allocation failure, including Release builds |
| Standard containers and remaining C++/third-party allocations | Their existing allocator failure policy applies; no recoverable OOM guarantee |

An `RtResult` return type alone does not make an entire operation recoverable:
metadata resolution may enter the infallible metadata/container domain. OOM
recovery tests warm metadata first, then inject failures in explicitly fallible
paths. This policy does not promise transactional recovery from arbitrary failures
halfway through runtime initialization or metadata publication.

## Verification

`foundation` tests size boundaries, bounded subreaders, physical array bounds,
inline/heap scratch ownership, replacement failures, scope-exit moves and callback
identity locking. `invocation-buffers` and `allocation-failures` install one
allocator before startup and inject failures by changing its state, never by
replacing the allocator under live objects. Their cases cover partial invocation
buffers and failed assembly-pool construction/rollback, followed by recovery.

String duplication and concatenation expose `StringUtil::try_*` APIs that return
`OutOfMemory` for allocation or size overflow. Null input retains optional-string
semantics. Legacy pointer-returning wrappers use the fatal allocation policy;
callers that need recovery must use the fallible APIs. Returned buffers belong to
the caller and are released with `GeneralAllocation::free`.

## Alignment

Ordinary memory callbacks must provide at least `alignof(std::max_align_t)`.
`GeneralAllocator<T>` and typed `GeneralAllocation` helpers use aligned callbacks
for types with greater alignment. Pair typed raw allocations with `free_any<T>`;
`delete_any<T>` also selects the matching release callback. Raw `malloc`/`free`
and `aligned_malloc`/`aligned_free` remain distinct allocation domains.

`MemPool` aligns absolute addresses inside its ordinary backing regions and
checks padding capacity before rollover. Requested power-of-two alignments do
not depend on the region base alignment; size and padding overflow return null.

## Standard container policy

Runtime HashMap/HashSet buckets and nodes use `StlAllocator`, an explicitly fatal
allocator backed by GeneralAllocation. It never returns null to a standard
container. Metadata and GC registries remain in the infallible domain; an
RtResult caller entering those caches does not promise arbitrary OOM recovery.
Global registries construct lazily on the runtime thread, so debug STL bookkeeping
cannot allocate and lock callbacks before the host configures them.

The GC handle index instead uses a fallible open-addressed table. Growth failure
leaves live handles intact and returns null before consuming a new ID or unlinking
a recycled handle. No standard-container allocation occurs on that path.
Profiling exports, standard strings and third-party library allocations remain
outside this container migration; the callback contract does not intercept global
operator new or the entire C/C++ library heap.
