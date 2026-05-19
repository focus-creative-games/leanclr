#pragma once

#include "rt_managed_types.h"
#include "metadata/rt_metadata.h"
#include "interp/interp_defs.h"
#include "gc/garbage_collector.h"
#include "class.h"

namespace leanclr
{
namespace vm
{
class Array
{
  public:
    // Array creation methods
    static size_t get_array_allocation_size(const metadata::RtClass* klass, int32_t length);

    static RtResult<RtArray*> __new_empty_szarray_by_ele_klass(const metadata::RtClass* ele_class LEANCLR_GC_DECLARE_CALL_SITE_PARAM)
    {
        return __new_szarray_from_ele_klass(ele_class, 0 LEANCLR_GC_CALL_SITE_PARAM);
    }

    static RtResult<RtArray*> __new_szarray_from_array_klass(const metadata::RtClass* klass, int32_t length LEANCLR_GC_DECLARE_CALL_SITE_PARAM);
    static RtResult<RtArray*> __new_szarray_from_ele_klass(const metadata::RtClass* ele_class, int32_t length LEANCLR_GC_DECLARE_CALL_SITE_PARAM);
    static RtResult<RtArray*> __new_mdarray_from_array_klass(const metadata::RtClass* arr_klass, const int32_t* lengths,
                                                             const int32_t* lower_bounds LEANCLR_GC_DECLARE_CALL_SITE_PARAM);
    static RtResult<RtArray*> __new_mdarray_from_ele_klass(const metadata::RtClass* ele_klass, int32_t rank, const int32_t* lengths,
                                                           const int32_t* lower_bounds LEANCLR_GC_DECLARE_CALL_SITE_PARAM);

    // Array information methods
    static int32_t get_array_length(const RtArray* array)
    {
        assert(array);
        return array->length;
    }

    static size_t get_array_byte_length(const RtArray* array)
    {
        assert(array);
        size_t ele_size = get_array_element_size(array);
        int32_t length = get_array_length(array);
        return ele_size * static_cast<size_t>(length);
    }

    static size_t get_array_element_size(const RtArray* array)
    {
        assert(array);
        const metadata::RtClass* ele_class = get_array_element_class(array);
        return Class::get_stack_location_size(ele_class);
    }

    static size_t get_array_element_size_by_klass(const metadata::RtClass* array_klass)
    {
        assert(array_klass && array_klass->element_class);
        return Class::get_stack_location_size(array_klass->element_class);
    }

    static const metadata::RtClass* get_array_element_class(const RtArray* array)
    {
        assert(array);
        return array->klass->element_class;
    }

    // Array index validation
    static bool is_valid_index(const RtArray* array, int32_t index)
    {
        assert(array);
        int32_t length = get_array_length(array);
        return (uint32_t)index < (uint32_t)length;
    }

    static bool is_out_of_range(const RtArray* array, int32_t index)
    {
        return !is_valid_index(array, index);
    }

    // Array data access methods
    template <typename T>
    static T* get_array_data_start_as(RtArray* array)
    {
        assert(array);
        return reinterpret_cast<T*>(const_cast<uint64_t*>(&array->first_data));
    }

    static void* get_array_data_start_as_ptr_void(RtArray* array)
    {
        assert(array);
        return &array->first_data;
    }

    template <typename T>
    static T* get_array_element_address(RtArray* array, int32_t index)
    {
        assert(array);
        assert(get_array_element_size(array) == sizeof(T));
        return reinterpret_cast<T*>(&array->first_data) + static_cast<size_t>(index);
    }

    static void* get_array_element_address_as_ptr_void(RtArray* array, int32_t index)
    {
        assert(array);
        assert(index >= 0 && index < get_array_length(array));
        size_t ele_size = get_array_element_size(array);
        return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&array->first_data) + ele_size * static_cast<size_t>(index));
    }

    static void* get_array_element_address_with_size_as_ptr_void(RtArray* array, int32_t index, size_t ele_size)
    {
        assert(array);
        assert(index >= 0 && index < get_array_length(array));
        return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(&array->first_data) + ele_size * static_cast<size_t>(index));
    }

    template <typename T>
    static T get_array_data_at(const RtArray* array, int32_t index)
    {
        assert(array);
        assert(get_array_element_size(array) == sizeof(T));
        const T* data_ptr = reinterpret_cast<const T*>(&array->first_data) + static_cast<size_t>(index);
        return *data_ptr;
    }

    template <typename T>
    static void set_array_data_at(RtArray* array, int32_t index, T value)
    {
        assert(array);
        assert(get_array_element_size(array) == sizeof(T));
        T* data_ptr = reinterpret_cast<T*>(&array->first_data) + static_cast<size_t>(index);
        *data_ptr = value;
    }

    static void copy_array_data_to_no_eval_stack(const RtArray* arr, int32_t start_index, void* dest);

    // Multi-dimensional array methods
    static RtResult<int32_t> get_array_length_at_dimension(const RtArray* array, size_t dimension);
    static RtResult<int32_t> get_array_lower_bound_at_dimension(const RtArray* array, size_t dimension);
    static RtResult<int32_t> get_global_index_from_indices(const RtArray* arr, RtArray* indices);
    static RtResult<int32_t> get_mdarray_global_index_from_indices2(const RtArray* arr, const interp::RtStackObject* indices);
    static RtResult<int32_t> get_mdarray_global_index_from_indices3(const RtArray* arr, int32_t* indices);

    // Method invoker implementations
    static RtResultVoid szarray_new_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid szarray_get_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid szarray_set_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid szarray_address_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid newmdarray_lengths_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                   const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid newmdarray_lengths_lower_bounds_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid mdarray_get_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid mdarray_set_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;
    static RtResultVoid mdarray_address_invoker(metadata::RtManagedMethodPointer method_pointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept;

    // Array cloning
    static RtResult<RtArray*> __clone(RtArray* old_arr LEANCLR_GC_DECLARE_CALL_SITE_PARAM) noexcept;
};

#if LEANCLR_GC_DEBUG
#define LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, site) ::leanclr::vm::Array::__new_empty_szarray_by_ele_klass(arr_klass, site)
#define LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, site) ::leanclr::vm::Array::__new_szarray_from_array_klass(arr_klass, length, site)
#define LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, site) ::leanclr::vm::Array::__new_szarray_from_ele_klass(ele_class, length, site)
#define LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, site) \
    ::leanclr::vm::Array::__new_mdarray_from_array_klass(arr_klass, lengths, lower_bounds, site)
#define LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, site) \
    ::leanclr::vm::Array::__new_mdarray_from_ele_klass(ele_klass, rank, lengths, lower_bounds, site)
#define LEANCLR_ARRAY_CLONE(arr, site) ::leanclr::vm::Array::__clone(arr, site)
#else
#define LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, site) ::leanclr::vm::Array::__new_empty_szarray_by_ele_klass(arr_klass)
#define LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, site) ::leanclr::vm::Array::__new_szarray_from_array_klass(arr_klass, length)
#define LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, site) ::leanclr::vm::Array::__new_szarray_from_ele_klass(ele_class, length)
#define LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, site) \
    ::leanclr::vm::Array::__new_mdarray_from_array_klass(arr_klass, lengths, lower_bounds)
#define LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, site) \
    ::leanclr::vm::Array::__new_mdarray_from_ele_klass(ele_klass, rank, lengths, lower_bounds)
#define LEANCLR_ARRAY_CLONE(arr, site) ::leanclr::vm::Array::__clone(arr)
#endif

#define LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS_INTERNAL(arr_klass, native_method) \
    LEANCLR_NEW_EMPTY_SZARRAY_BY_ELE_KLASS(arr_klass, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
#define LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS_INTERNAL(arr_klass, length, native_method) \
    LEANCLR_NEW_SZARRAY_FROM_ARRAY_KLASS(arr_klass, length, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
#define LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(ele_class, length, native_method) \
    LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS(ele_class, length, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
#define LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS_INTERNAL(arr_klass, lengths, lower_bounds, native_method) \
    LEANCLR_NEW_MDARRAY_FROM_ARRAY_KLASS(arr_klass, lengths, lower_bounds, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
#define LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS_INTERNAL(ele_klass, rank, lengths, lower_bounds, native_method) \
    LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(ele_klass, rank, lengths, lower_bounds, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
#define LEANCLR_ARRAY_CLONE_INTERNAL(arr, native_method) LEANCLR_ARRAY_CLONE(arr, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_method))
} // namespace vm
} // namespace leanclr
