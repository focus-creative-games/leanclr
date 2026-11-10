#include "public/leanclr.h"
int main()
{
    // Verify both the C pointer signature and the legacy C++ overload link.
    using Legacy = void (*)(LeanclrModuleDef*, bool, LeanclrClass**&, size_t&, LeanclrException**);
    Legacy volatile enumerate = static_cast<Legacy>(leanclr_get_classes);
    (void)enumerate;
    return leanclr_get_assembly_count() == 0 ? 0 : 1;
}
