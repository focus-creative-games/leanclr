#pragma once
#include "rt_managed_types.h"
#include "utils/checked_size.h"
#include <cstddef>

namespace leanclr
{
namespace vm
{
struct ArrayLayout
{
    size_t bounds_offset;
    size_t allocation_size;

    static bool try_compute(size_t element_size, size_t length, size_t rank, ArrayLayout& result)
    {
        size_t data_bytes, data_end;
        if (!utils::CheckedSize::multiply(element_size, length, data_bytes) || !utils::CheckedSize::add(offsetof(RtArray, first_data), data_bytes, data_end))
            return false;
        ArrayLayout layout = {0, data_end};
        if (!utils::CheckedSize::align_up(data_end, 8, layout.bounds_offset))
            return false;
        if (rank != 0)
        {
            size_t bounds_bytes;
            if (!utils::CheckedSize::multiply(rank, sizeof(ArrayBounds), bounds_bytes) ||
                !utils::CheckedSize::add(layout.bounds_offset, bounds_bytes, layout.allocation_size))
                return false;
        }
        result = layout;
        return true;
    }
};
} // namespace vm
} // namespace leanclr
