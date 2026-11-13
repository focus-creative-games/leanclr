#pragma once

#include "alloc/scoped_buffer.h"
#include "vm/class.h"
#include "vm/method.h"

namespace leanclr
{
namespace vm
{
// Enumerate from derived to base. Slots, rather than names/signatures, distinguish
// overrides from new virtuals and overloads. The bitmap costs one bit per slot.
class ReflectionMethodSlots
{
    alloc::ScopedBuffer<uint8_t, 64> seen;
    uint16_t slot_count = 0;

  public:
    RtResultVoid initialize(metadata::RtClass* klass, bool declared_only)
    {
        if (declared_only)
            RET_VOID_OK();
        RET_ERR_ON_FAIL(Class::initialize_vtables(klass));
        slot_count = klass->vtable_count;
        if (!seen.try_allocate_zeroed((static_cast<size_t>(slot_count) + 7) / 8))
            RET_ERR(RtErr::OutOfMemory);
        RET_VOID_OK();
    }

    bool include(const metadata::RtMethodInfo* method)
    {
        if (!Method::is_virtual(method) || method->slot == metadata::RT_INVALID_METHOD_SLOT || method->slot >= slot_count)
            return true;
        auto& byte = seen.data()[method->slot / 8];
        const auto mask = static_cast<uint8_t>(1u << (method->slot % 8));
        if (byte & mask)
            return false;
        byte |= mask;
        return true;
    }
};
} // namespace vm
} // namespace leanclr
