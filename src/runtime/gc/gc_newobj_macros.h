#pragma once

#include "gc/garbage_collector.h"
#include "gc/gc_config.h"

// Allocation macros: when LEANCLR_GC_DEBUG is off, site parameters are not referenced so
// __FILE__ / __LINE__ / method strings are not retained in the generated binary.

#if LEANCLR_GC_DEBUG

#define LEANCLR_CODEGEN_NEWOBJ(klass, file, line, method)                                                              \
    ::leanclr::gc::GarbageCollector::allocate_object_with_site((klass), ::leanclr::gc::gc_object_size(klass),          \
                                                                ::leanclr::gc::GcAllocSite::make_codegen((file), (line), (method)))

#define LEANCLR_CODEGEN_NEWARRAY(arr_klass, total_bytes, file, line, method)                                            \
    ::leanclr::gc::GarbageCollector::allocate_array_with_site((arr_klass), (total_bytes),                              \
                                                              ::leanclr::gc::GcAllocSite::make_codegen((file), (line), (method)))

#define LEANCLR_NEWOBJ_INTERP(klass, method, il_offset)                                                                \
    ::leanclr::gc::GarbageCollector::allocate_object_with_site((klass), ::leanclr::gc::gc_object_size(klass),          \
                                                                ::leanclr::gc::GcAllocSite::make_interp((method), (il_offset)))

#define LEANCLR_NEWARRAY_INTERP(arr_klass, total_bytes, method, il_offset)                                             \
    ::leanclr::gc::GarbageCollector::allocate_array_with_site((arr_klass), (total_bytes),                              \
                                                              ::leanclr::gc::GcAllocSite::make_interp((method), (il_offset)))

#define LEANCLR_NEWOBJ_INTERNAL(klass, file, line, native_runtime_method)                                              \
    ::leanclr::gc::GarbageCollector::allocate_object_with_site((klass), ::leanclr::gc::gc_object_size(klass),          \
                                                                ::leanclr::gc::GcAllocSite::make_internal((file), (line), (native_runtime_method)))

#define LEANCLR_NEWOBJ_INTERNAL_SIZE(klass, size, file, line, native_runtime_method)                                   \
    ::leanclr::gc::GarbageCollector::allocate_object_with_site((klass), (size),                                        \
                                                                ::leanclr::gc::GcAllocSite::make_internal((file), (line), (native_runtime_method)))

#define LEANCLR_NEWOBJ_INTERNAL_NOREF(klass, size, file, line, native_runtime_method)                                \
    ::leanclr::gc::GarbageCollector::allocate_object_not_contains_references_with_site(                              \
        (klass), (size), ::leanclr::gc::GcAllocSite::make_internal((file), (line), (native_runtime_method)))

#define LEANCLR_NEWARRAY_INTERNAL(arr_klass, total_bytes, file, line, native_runtime_method)                           \
    ::leanclr::gc::GarbageCollector::allocate_array_with_site((arr_klass), (total_bytes),                              \
                                                              ::leanclr::gc::GcAllocSite::make_internal((file), (line), (native_runtime_method)))

#else

#define LEANCLR_CODEGEN_NEWOBJ(klass, file, line, method)                                                              \
    ::leanclr::gc::GarbageCollector::allocate_object((klass), ::leanclr::gc::gc_object_size(klass))

#define LEANCLR_CODEGEN_NEWARRAY(arr_klass, total_bytes, file, line, method)                                            \
    ::leanclr::gc::GarbageCollector::allocate_array((arr_klass), (total_bytes))

#define LEANCLR_NEWOBJ_INTERP(klass, method, il_offset)                                                                \
    ::leanclr::gc::GarbageCollector::allocate_object((klass), ::leanclr::gc::gc_object_size(klass))

#define LEANCLR_NEWARRAY_INTERP(arr_klass, total_bytes, method, il_offset)                                             \
    ::leanclr::gc::GarbageCollector::allocate_array((arr_klass), (total_bytes))

#define LEANCLR_NEWOBJ_INTERNAL(klass, file, line, native_runtime_method)                                              \
    ::leanclr::gc::GarbageCollector::allocate_object((klass), ::leanclr::gc::gc_object_size(klass))

#define LEANCLR_NEWOBJ_INTERNAL_SIZE(klass, size, file, line, native_runtime_method)                                   \
    ::leanclr::gc::GarbageCollector::allocate_object((klass), (size))

#define LEANCLR_NEWOBJ_INTERNAL_NOREF(klass, size, file, line, native_runtime_method)                                \
    ::leanclr::gc::GarbageCollector::allocate_object_not_contains_references((klass), (size))

#define LEANCLR_NEWARRAY_INTERNAL(arr_klass, total_bytes, file, line, native_runtime_method)                           \
    ::leanclr::gc::GarbageCollector::allocate_array((arr_klass), (total_bytes))

#endif
