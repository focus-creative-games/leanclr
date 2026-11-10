#include "host/services.h"
#include "public/leanclr.h"

int main()
{
    // Verify both the C pointer signature and the legacy C++ overload link.
    using Legacy = void (*)(LeanclrModuleDef*, bool, LeanclrClass**&, size_t&, LeanclrException**);
    Legacy volatile enumerate = static_cast<Legacy>(leanclr_get_classes);
    (void)enumerate;
    // Host contracts must also be usable by an external DLL consumer.
    auto services = leanclr::host::Services::get_callbacks();
    if (!services.utc_ticks_100ns || !services.files.open || !services.random_bytes)
        return 2;
    leanclr::host::ServiceCallbacks overrides = {};
    if (!leanclr::host::Services::configure(overrides))
        return 3;
    return leanclr_get_assembly_count() == 0 ? 0 : 1;
}
