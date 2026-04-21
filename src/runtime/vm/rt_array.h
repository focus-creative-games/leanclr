#pragma once

#include "rt_managed_types.h"
#include "metadata/rt_metadata.h"
#include "interp/interp_defs.h"

namespace leanclr
{
namespace vm
{
class Array
{
  public:
    // Array creation methods
    static size_t get_array_allocation_size(const metadata::RtClass* klass, int32_t length);
    static RtResult<RtArray*> new_empty_szarray_by_ele_klass(const metadata::RtClass* ele_class);
    static RtResult<RtArray*> new_szarray_from_array_klass(const metadata::RtClass* klass, int32_t length);
    static RtResult<RtArray*> new_szarray_from_ele_klass(const metadata::RtClass* ele_class, int32_t length);
    static RtResult<RtArray*> new_mdarray_from_array_klass(const metadata::RtClass* arr_klass, const int32_t* lengths, const int32_t* lower_bounds);
    static RtResult<RtArray*> new_mdarray_from_ele_klass(const metadata::RtClass* ele_klass, int32_t rank, const int32_t* lengths, const int32_t* lower_bounds);

    // Array information methods
    static int32_t get_array_length(const RtArray* array)
    {
        assert(array);
        return array->length;
    }

    static size_t get_array_byte_length(const RtArray* array);
    static size_t get_array_element_size(const RtArray* array);
    static size_t get_array_element_size_by_klass(const metadata::RtClass* array_klass);
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
    static void* get_array_data_start_as_ptr_void(RtArray* array);

    template <typename T>
    static T* get_array_element_address(RtArray* array, int32_t index)
    {
        assert(array);
        assert(get_array_element_size(array) == sizeof(T));
        return reinterpret_cast<T*>(&array->first_data) + static_cast<size_t>(index);
    }
    static void* get_array_element_address_as_ptr_void(RtArray* array, int32_t index);
    static void* get_array_element_address_with_size_as_ptr_void(RtArray* array, int32_t index, size_t ele_size);

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
    static RtResult<RtArray*> clone(RtArray* old_arr) noexcept;
};
} // namespace vm
} // namespace leanclr
