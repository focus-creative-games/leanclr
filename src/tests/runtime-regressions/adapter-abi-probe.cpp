#include "bcl/mono/managed_layouts.h"

leanclr::RtResult<leanclr::vm::RtException*> adapter_result(leanclr::vm::RtException* object, bool fail)
{
    if (fail)
        RET_ERR(leanclr::RtErr::InvalidCast);
    RET_OK(object);
}
