#include "interp/basic_block_splitter.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
using namespace leanclr;

static void check(bool condition)
{
    if (!condition)
        std::abort();
}

static void split(const uint8_t* bytes, size_t count, bool valid)
{
    metadata::RtMethodBody body = {};
    body.code = bytes;
    body.code_size = static_cast<uint32_t>(count);
    alloc::MemPool pool;
    interp::BasicBlockSplitter splitter(&body, &pool);
    auto result = splitter.split();
    if (valid)
        check(result.is_ok());
    else
        check(result.is_err() && result.unwrap_err() == RtErr::BadImageFormat);
}

int main()
{
    const uint8_t good[] = {0x45, 1, 0, 0, 0, 0, 0, 0, 0, 0x2a};
    const interp::il::OpCodeInfo* op = nullptr;
    check(interp::il::OpCodes::try_decode_opcode_info(good, good + sizeof(good), op));
    for (size_t n = 1; n < 9; ++n)
    {
        size_t size = 123;
        check(!interp::il::OpCodes::try_get_opcode_size(good, good + n, op, size) && size == 123);
        split(good, n, false);
    }
    split(good, sizeof(good), true);
    const uint8_t huge[] = {0x45, 0xff, 0xff, 0xff, 0xff};
    split(huge, sizeof(huge), false);
    const uint8_t prefix[] = {0xfe};
    split(prefix, sizeof(prefix), false);
    const uint8_t ldc[] = {0x21, 0, 0};
    split(ldc, sizeof(ldc), false);
    const uint8_t forward[] = {0x38, 0xff, 0xff, 0xff, 0x7f, 0x2a};
    split(forward, sizeof(forward), false);
    const uint8_t backward[] = {0x38, 0, 0, 0, 0x80, 0x2a};
    split(backward, sizeof(backward), false);
    const uint8_t middle[] = {0x2b, 1, 0x20, 0, 0, 0, 0, 0x2a};
    split(middle, sizeof(middle), false);
    const uint8_t loop[] = {0x2b, 0xfe};
    split(loop, sizeof(loop), true);
    const uint8_t bad_switch[] = {0x45, 1, 0, 0, 0, 0xff, 0xff, 0xff, 0x7f, 0x2a};
    split(bad_switch, sizeof(bad_switch), false);
    metadata::RtMethodBody body = {};
    const uint8_t ret[] = {0x2a};
    body.code = ret;
    body.code_size = 1;
    metadata::RtExceptionClause clause = {};
    clause.try_offset = UINT32_MAX;
    clause.try_length = 2;
    body.exception_clauses.push_back(clause);
    alloc::MemPool pool;
    interp::BasicBlockSplitter splitter(&body, &pool);
    auto error = splitter.split();
    check(error.is_err() && error.unwrap_err() == RtErr::BadImageFormat);
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    auto* pages = static_cast<uint8_t*>(VirtualAlloc(nullptr, info.dwPageSize * 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    check(pages != nullptr);
    DWORD old;
    check(VirtualProtect(pages + info.dwPageSize, info.dwPageSize, PAGE_NOACCESS, &old) != 0);
    for (size_t n = 1; n < 9; ++n)
    {
        auto* p = pages + info.dwPageSize - n;
        std::memcpy(p, good, n);
        split(p, n, false);
    }
    VirtualFree(pages, 0, MEM_RELEASE);
#endif
    std::puts("IL bounds passed");
}
