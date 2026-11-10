#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "alloc/general_allocation.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/class.h"
#include "vm/rt_array.h"
#include "vm/gc.h"
#include "vm/gchandle.h"

using namespace leanclr;
static std::string dll_dir;
static void check(bool condition, const char* message)
{
    if (!condition) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
static bool load_file(const char* name, const char* ext, vm::FileData& output)
{
    for (const char* sub : {"/", "/mono-4.5/"})
    {
        std::ifstream input(dll_dir + sub + name + "." + ext, std::ios::binary | std::ios::ate);
        if (!input) continue;
        auto length = input.tellg();
        if (length < 0) return false;
        auto* data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
        if (!data) return false;
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(data), length)) { alloc::GeneralAllocation::free(data); return false; }
        output = {data, static_cast<size_t>(length), false};
        return true;
    }
    return false;
}
static void zero_arrays()
{
    auto* element = vm::Class::get_corlib_types().cls_int32;
    const int32_t dimensions[][3] = {{0,1,2}, {1,0,2}, {1,2,0}, {0,0,0}, {INT32_MAX,INT32_MAX,0}};
    const int32_t lower[] = {-2, -3, 7};
    for (auto& dims : dimensions)
    {
        auto result = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 3, dims, lower, gc::GcAllocSite::none());
        check(result.is_ok(), "empty array creation failed");
        auto* array = result.unwrap();
        check(array->length == 0, "empty array length");
        for (int i = 0; i < 3; ++i)
            check(array->bounds[i].length == dims[i] && array->bounds[i].lower_bound == lower[i], "empty array bounds");
    }
    const int32_t invalid[] = {0,-1};
    auto negative = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, invalid, nullptr, gc::GcAllocSite::none());
    check(negative.is_err() && negative.unwrap_err() == core::RtErr::Overflow, "zero must not hide negative dimension");
    const int32_t huge[] = {INT32_MAX,2};
    auto overflow = LEANCLR_NEW_MDARRAY_FROM_ELE_KLASS(element, 2, huge, nullptr, gc::GcAllocSite::none());
    check(overflow.is_err() && overflow.unwrap_err() == core::RtErr::Overflow, "array product overflow");
}
int main(int argc, char** argv)
{
    check(argc >= 3, "usage: runtime-regressions CASE DLL_DIRECTORY");
    dll_dir = argv[2];
    vm::Settings::set_file_loader(load_file);
    const char* args[] = {"runtime-regressions"};
    vm::Settings::set_command_line_arguments(1, args);
    vm::Settings::set_gc_mode(gc::GCMode::DISABLED);
    auto initialized = vm::Runtime::initialize();
    check(initialized.is_ok(), "runtime initialization failed");
    if (std::strcmp(argv[1], "zero-arrays") == 0) zero_arrays();
    else check(false, "unknown regression case");
    return 0;
}
