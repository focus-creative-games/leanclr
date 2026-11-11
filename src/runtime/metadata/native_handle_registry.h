#pragma once
#include "core/rt_base.h"

namespace leanclr
{
namespace metadata
{
class RtModuleDef;
enum class NativeHandleKind : uintptr_t
{
    Method = 0,
    Field = 1,
    Property = 2,
    Event = 3
};

// Exact identities only: lookup never dereferences an untrusted address.
class NativeHandleRegistry
{
  public:
    static bool insert(const void* pointer, NativeHandleKind kind);
    static bool contains(const void* pointer, NativeHandleKind kind);
    static void remove_module(const RtModuleDef* module);
};
} // namespace metadata
} // namespace leanclr
