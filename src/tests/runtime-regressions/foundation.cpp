#include "utils/checked_size.h"
#include "utils/scope_guard.h"
#include "alloc/scoped_buffer.h"
#include "utils/binary_reader.h"
#include "vm/array_layout.h"
#include "core/rt_base.h"
#include <limits>
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

static alloc::MemoryCallbacks original_callbacks;
static bool fail_allocations;
static size_t allocations, frees;

// Compile this specialization: its discarded branch must not instantiate the
// ambiguous value/error constructors of Result<RtErr>.
core::Result<core::RtErr> forward_error_payload(core::Result<core::RtErr>& source)
{
    return source.cast<core::RtErr>();
}

static void verify_result_casts()
{
    const int32_t values[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    for (int32_t value : values)
    {
        core::Result<int32_t> source(value);
        auto result = source.cast<int64_t>();
        check(result.is_ok() && result.unwrap() == value, "signed Result cast preserves payload");
    }
    core::Result<uint32_t> source(std::numeric_limits<uint32_t>::max());
    auto result = source.cast<uint64_t>();
    check(result.is_ok() && result.unwrap() == UINT32_MAX, "unsigned Result cast preserves full payload");
    for (unsigned code = 1; code <= UINT8_MAX; ++code)
    {
        const auto error = static_cast<core::RtErr>(code);
        core::Result<int32_t> failed(error);
        auto converted = failed.cast<uint64_t>();
        check(converted.is_err() && converted.unwrap_err() == error, "Result cast preserves error bytes");
    }
}

static void* test_calloc(size_t count, size_t size)
{
    if (fail_allocations)
        return nullptr;
    void* ptr = original_callbacks.calloc(count, size);
    if (ptr)
        ++allocations;
    return ptr;
}

static void test_free(void* ptr)
{
    ++frees;
    original_callbacks.free(ptr);
}

int main()
{
    verify_result_casts();
    original_callbacks = alloc::GeneralAllocation::get_memory_callbacks();
    auto invalid = original_callbacks;
    invalid.free = nullptr;
    check(!alloc::GeneralAllocation::set_memory_callbacks(invalid), "reject incomplete callback table");
    auto installed = original_callbacks;
    installed.calloc = test_calloc;
    installed.free = test_free;
    check(alloc::GeneralAllocation::set_memory_callbacks(installed), "install callbacks before allocation");
    check(alloc::GeneralAllocation::set_memory_callbacks(original_callbacks) && alloc::GeneralAllocation::set_memory_callbacks(installed),
          "callback configuration is mutable before first allocation");
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
    alloc::ScopedBuffer<uint64_t, 2> buffer;
    check(buffer.try_allocate_zeroed(2) && buffer.size() == 2 && buffer.data()[1] == 0, "inline scratch buffer");
    buffer.data()[0] = 42;
    auto* inline_data = buffer.data();
    check(!buffer.try_allocate_zeroed(SIZE_MAX) && buffer.data() == inline_data && buffer.data()[0] == 42, "scratch overflow preserves buffer");
    fail_allocations = true;
    check(!buffer.try_allocate_zeroed(20) && buffer.data() == inline_data && buffer.data()[0] == 42, "scratch allocator failure preserves inline contents");
    check(!alloc::GeneralAllocation::set_memory_callbacks(original_callbacks), "allocation attempt locks callback identity");
    check(alloc::GeneralAllocation::set_memory_callbacks(installed), "identical callback installation remains valid");
    fail_allocations = false;
    check(buffer.try_allocate_zeroed(20) && buffer.data() != inline_data && buffer.data()[19] == 0, "heap scratch buffer");
    auto* heap_data = buffer.data();
    heap_data[0] = 123;
    fail_allocations = true;
    check(!buffer.try_allocate_zeroed(40) && buffer.data() == heap_data && buffer.data()[0] == 123 && buffer.size() == 20,
          "scratch replacement failure preserves heap ownership");
    fail_allocations = false;
    check(buffer.try_allocate_zeroed(1) && buffer.data() == inline_data && buffer.data()[0] == 0, "heap to inline ownership transition");
    check(buffer.try_allocate_zeroed(0) && buffer.size() == 0, "empty scratch buffer");
    check(allocations == frees && allocations == 1, "callback allocation/free ownership balances");
    check(!alloc::GeneralAllocation::set_memory_callbacks(original_callbacks), "callbacks remain locked after buffers are freed");
    check(alloc::GeneralAllocation::aligned_malloc(32, 3) == nullptr && alloc::GeneralAllocation::aligned_realloc(nullptr, 32, 0) == nullptr,
          "invalid aligned requests fail without assertion");
    int cleanups = 0;
    {
        auto guard = utils::make_scope_exit([&cleanups]() { ++cleanups; });
        auto moved = std::move(guard);
        auto dismissed = utils::make_scope_exit([&cleanups]() { cleanups += 100; });
        dismissed.dismiss();
    }
    check(cleanups == 1, "moved cleanup runs once and dismissed cleanup never runs");
    std::puts("foundation tests passed");
}
