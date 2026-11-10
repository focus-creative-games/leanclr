#include "public/leanclr.h"
int main(void)
{
    void (*volatile enumerate)(LeanclrModuleDef*, bool, LeanclrClass***, size_t*, LeanclrException**) = leanclr_get_classes;
    (void)enumerate;
    return leanclr_get_assembly_count() == 0 ? 0 : 1;
}
