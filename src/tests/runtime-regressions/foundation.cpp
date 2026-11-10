#include "utils/checked_size.h"
#include "utils/binary_reader.h"
#include "vm/array_layout.h"
#include <cstdio>
#include <cstdlib>

using namespace leanclr;

static void check(bool value, const char* message)
{
    if (!value)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

int main()
{
    size_t value = 17;
    check(!utils::CheckedSize::add(SIZE_MAX, 1, value) && value == 17, "addition overflow preserves output");
    check(!utils::CheckedSize::multiply(SIZE_MAX, 2, value) && value == 17, "multiplication overflow preserves output");
    check(utils::CheckedSize::multiply(SIZE_MAX, 0, value) && value == 0, "zero product");
    check(utils::CheckedSize::add(SIZE_MAX - 1, 1, value) && value == SIZE_MAX, "maximum size");
    check(!utils::CheckedSize::align_up(SIZE_MAX, 8, value) && value == SIZE_MAX, "alignment overflow");
    check(!utils::CheckedSize::align_up(0, 0, value) && !utils::CheckedSize::align_up(0, 3, value), "invalid alignment");
    check(utils::CheckedSize::contains(8, 8, 0) && !utils::CheckedSize::contains(8, 9, 0) && !utils::CheckedSize::contains(8, 1, SIZE_MAX), "bounded ranges");
    const uint8_t bytes[] = {1, 2, 3, 4, 5, 6};
    utils::BinaryReader parent(bytes, sizeof(bytes)), child(nullptr, 0);
    check(parent.try_read_slice(3, child) && parent.get_position() == 3 && child.length() == 3, "bounded child reader");
    uint32_t word;
    check(!child.try_read_u32(word) && child.get_position() == 0, "child cannot consume sibling bytes");
    check(!parent.try_read_slice(4, child) && parent.get_position() == 3 && child.length() == 3, "failed slice is transactional");
    check(!parent.is_aligned(0), "zero alignment is rejected");
    vm::ArrayLayout layout;
    check(vm::ArrayLayout::try_compute(3, 3, 2, layout), "odd-sized MD array elements");
    const size_t data_end = offsetof(vm::RtArray, first_data) + 9;
    check(layout.bounds_offset == (data_end + 7) / 8 * 8 && layout.allocation_size == layout.bounds_offset + 2 * sizeof(vm::ArrayBounds),
          "shared bounds layout");
    check(vm::ArrayLayout::try_compute(SIZE_MAX, 0, 3, layout), "empty array ignores element multiplication");
    const auto previous = layout;
    check(!vm::ArrayLayout::try_compute(SIZE_MAX, 2, 1, layout) && layout.allocation_size == previous.allocation_size, "array size overflow");
    check(!vm::ArrayLayout::try_compute(1, 0, SIZE_MAX, layout), "bounds size overflow");
    std::puts("foundation tests passed");
}
