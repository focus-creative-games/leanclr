#include "host/services.h"
#include "public/leanclr.h"

#if defined(LEANCLR_EXPECT_CPP20)
#if defined(_MSVC_LANG)
static_assert(_MSVC_LANG >= 202002L, "The consumer must actually compile as C++20");
#else
static_assert(__cplusplus >= 202002L, "The consumer must actually compile as C++20");
#endif
#endif

int main()
{
    // Verify both the C pointer signature and the legacy C++ overload link.
    using Legacy = void (*)(LeanclrModuleDef*, bool, LeanclrClass**&, size_t&, LeanclrException**);
    Legacy volatile enumerate = static_cast<Legacy>(leanclr_get_classes);
    (void)enumerate;
    // Host contracts must also be usable by an external DLL consumer.
    auto services = leanclr::host::Services::get_callbacks();
#if !defined(LEANCLR_CUSTOM_HOST)
    if (!services.utc_ticks_100ns || !services.files.open || !services.random_bytes)
        return 2;
#endif
    leanclr::host::ServiceCallbacks overrides = {};
    if (!leanclr::host::Services::configure(overrides))
        return 3;
    return leanclr_get_assembly_count() == 0 ? 0 : 1;
}
