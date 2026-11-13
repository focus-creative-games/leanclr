#include "calli.h"
#include "alloc/general_allocation.h"
#include "utils/encode_conv.h"

namespace leanclr
{
namespace vm
{
namespace
{
// Open addressing gives fallible growth and allocation-free repeated ldftn/calli.
// Only escaped managed targets occupy this process-owned metadata index.
const metadata::RtMethodInfo** targets = nullptr;
size_t capacity = 0, count = 0;

size_t slot_for(const metadata::RtMethodInfo* method, size_t size)
{
    auto key = reinterpret_cast<uintptr_t>(method) / alignof(metadata::RtMethodInfo);
    key ^= key >> 16;
    key *= uintptr_t(0x9e3779b1u);
    return static_cast<size_t>(key) & (size - 1);
}

bool contains(const metadata::RtMethodInfo* method)
{
    if (!method || !capacity)
        return false;
    auto slot = slot_for(method, capacity);
    while (targets[slot])
    {
        if (targets[slot] == method)
            return true;
        slot = (slot + 1) & (capacity - 1);
    }
    return false;
}
} // namespace

RtResultVoid Calli::register_managed_target(const metadata::RtMethodInfo* method)
{
    if (!method)
        RET_ERR(RtErr::ArgumentNull);
    if (contains(method))
        RET_VOID_OK();
    if (!capacity || count >= capacity - capacity / 4)
    {
        if (capacity > SIZE_MAX / 2 / sizeof(*targets))
            RET_ERR(RtErr::OutOfMemory);
        const size_t next = capacity ? capacity * 2 : 16;
        auto* replacement = alloc::GeneralAllocation::calloc_any<const metadata::RtMethodInfo*>(next);
        if (!replacement)
            RET_ERR(RtErr::OutOfMemory);
        for (size_t i = 0; i < capacity; ++i)
        {
            if (!targets[i])
                continue;
            auto slot = slot_for(targets[i], next);
            while (replacement[slot])
                slot = (slot + 1) & (next - 1);
            replacement[slot] = targets[i];
        }
        alloc::GeneralAllocation::free(targets);
        targets = replacement;
        capacity = next;
    }
    auto slot = slot_for(method, capacity);
    while (targets[slot])
        slot = (slot + 1) & (capacity - 1);
    targets[slot] = method;
    ++count;
    RET_VOID_OK();
}

bool Calli::is_managed_target(const metadata::RtMethodInfo* target)
{
    return contains(target);
}

RtResultVoid Calli::invoke_native(const metadata::RtCalliBridge* bridge, void* target, interp::RtStackObject* args, interp::RtStackObject* ret)
{
    if (!target)
        RET_ERR(RtErr::NullReference);
    if (contains(reinterpret_cast<const metadata::RtMethodInfo*>(target)))
        RET_ERR(RtErr::Argument);
    if (!bridge || !bridge->invoke || !bridge->return_type)
        RET_ERR(RtErr::NotSupported);
    if ((bridge->parameter_count && !args) || (bridge->return_type->kind != 1 && !ret))
        RET_ERR(RtErr::ArgumentNull);
    bridge->invoke(target, args, ret);
    RET_VOID_OK();
}

uint8_t Calli::char_to_native(uint16_t value)
{
    Utf16Char input = static_cast<Utf16Char>(value);
    char bytes[8]{};
    size_t count = 0;
    utils::EncodeConv::utf16_to_ansi(&input, 1, bytes, count);
    return count ? static_cast<uint8_t>(bytes[0]) : uint8_t('?');
}

uint16_t Calli::char_from_native(uint8_t value)
{
    char input = static_cast<char>(value);
    Utf16Char chars[4]{};
    size_t count = 0;
    utils::EncodeConv::ansi_to_utf16(&input, 1, chars, count);
    return count ? static_cast<uint16_t>(chars[0]) : uint16_t(0xfffd);
}

} // namespace vm
} // namespace leanclr
