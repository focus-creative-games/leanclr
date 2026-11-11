#include "utils/mem_op.h"

int main()
{
    for (size_t i = 0; i < 64; ++i)
    {
        const uint64_t bit = uint64_t(1) << i;
        if (leanclr::utils::MemOp::count_trailing_zeros_nonzero64(bit) != i ||
            leanclr::utils::MemOp::count_trailing_zeros_nonzero64(bit | (uint64_t(1) << 63)) != i)
            return 1;
        if (i < sizeof(size_t) * 8 && leanclr::utils::MemOp::count_trailing_zeros_nonzero(static_cast<size_t>(bit)) != i)
            return 2;
    }
    return 0;
}
