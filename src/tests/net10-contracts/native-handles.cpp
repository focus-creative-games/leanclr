#include "../aot-returns/test_assert.h"
#include "metadata/native_handle_registry.h"
#include "metadata/rt_metadata.h"
#include <cstdlib>
using namespace leanclr;

static void check(bool value)
{
    if (!value)
        std::abort();
}

int main()
{
    configure_test_assertions();
    metadata::RtClass first{}, second{};
    first.image = reinterpret_cast<metadata::RtModuleDef*>(uintptr_t{0x1000});
    second.image = reinterpret_cast<metadata::RtModuleDef*>(uintptr_t{0x2000});
    // Keep the stress fixture off the default 64 KiB Wasm stack.
    static metadata::RtMethodInfo methods[1024]{};
    for (size_t i = 0; i < 1024; ++i)
    {
        methods[i].parent = i % 2 ? &first : &second;
        check(metadata::NativeHandleRegistry::insert(&methods[i], metadata::NativeHandleKind::Method));
        check(metadata::NativeHandleRegistry::insert(&methods[i], metadata::NativeHandleKind::Method));
    }
    for (const auto& method : methods)
    {
        check(metadata::NativeHandleRegistry::contains(&method, metadata::NativeHandleKind::Method));
        check(!metadata::NativeHandleRegistry::contains(&method, metadata::NativeHandleKind::Field));
    }
    metadata::NativeHandleRegistry::remove_module(first.image);
    for (size_t i = 0; i < 1024; ++i)
        check(metadata::NativeHandleRegistry::contains(&methods[i], metadata::NativeHandleKind::Method) == (i % 2 == 0));
    metadata::NativeHandleRegistry::remove_module(second.image);
    for (const auto& method : methods)
        check(!metadata::NativeHandleRegistry::contains(&method, metadata::NativeHandleKind::Method));
    check(!metadata::NativeHandleRegistry::contains(reinterpret_cast<void*>(uintptr_t{1}), metadata::NativeHandleKind::Method));
    check(!metadata::NativeHandleRegistry::insert(nullptr, metadata::NativeHandleKind::Method));
    return 0;
}
