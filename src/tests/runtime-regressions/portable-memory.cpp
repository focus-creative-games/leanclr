#include "utils/binary_reader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace leanclr::utils;

static void check(bool value)
{
    if (!value)
        std::abort();
}

int main()
{
    const uint8_t encoded[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, 0, 0, 0xC0, 0x3F, 0, 0, 0, 0, 0, 0, 0x04, 0xC0};
    alignas(8) uint8_t buffer[64] = {};
    for (size_t offset = 0; offset < 8; ++offset)
    {
        std::memcpy(buffer + offset, encoded, sizeof(encoded));
        BinaryReader reader(buffer + offset, sizeof(encoded));
        uint64_t integer;
        float single;
        double real;
        check(reader.try_read_u64(integer) && integer == UINT64_C(0x0123456789ABCDEF));
        check(reader.try_read_f32(single) && single == 1.5f);
        check(reader.try_read_f64(real) && real == -2.5);
        check(LittleEndian::read_u16(buffer + offset) == 0xCDEF);
        check(LittleEndian::read_u32(buffer + offset) == 0x89ABCDEF);
        check(LittleEndian::read_i32(buffer + offset) == -1985229329);
        check(!reader.try_read_u64(integer));

        MemOp::write_i64_may_unaligned(buffer + offset, INT64_MIN + 123);
        check(MemOp::read_i64_may_unaligned(buffer + offset) == INT64_MIN + 123);
        auto* native = reinterpret_cast<double*>(buffer + offset);
        MemOp::store_native(native, -3.25);
        check(MemOp::load_native(native) == -3.25);
        MemOp::write_i32_may_unaligned(buffer + offset, -100001);
        check(MemOp::read_i32_may_unaligned(buffer + offset) == -100001);
        MemOp::write_i16_may_unaligned(buffer + offset, -12345);
        check(MemOp::read_i16_may_unaligned(buffer + offset) == -12345);
        int marker = 0;
        MemOp::write_ptr_may_unaligned(buffer + offset, &marker);
        check(MemOp::read_ptr_may_unaligned(buffer + offset) == &marker);
    }
    int values[] = {1, 2, 3, 4, 5};
    MemOp::copy_obj_array_mayoverlapping(values + 1, values, 4);
    const int expected[] = {1, 1, 2, 3, 4};
    check(std::memcmp(values, expected, sizeof(values)) == 0);
    MemOp::copy_obj_array_mayoverlapping(values, values + 1, 4);
    const int expected_back[] = {1, 2, 3, 4, 4};
    check(std::memcmp(values, expected_back, sizeof(values)) == 0);
    BinaryReader unaligned(buffer + 1, 8);
    const uint64_t* typed;
    check(!unaligned.try_peek_any_ptr(typed));
    check(!unaligned.try_peek_any_ptr_range(1, typed));
    std::puts("portable memory passed");
}
