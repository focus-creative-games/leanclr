#pragma once

#include "metadata/rt_metadata.h"
#include "utils/rt_vector.h"
#include "vm/internal_calls.h"

namespace leanclr
{
namespace icalls
{
class InternalCallStubs
{
  public:
    static void get_internal_call_entries(utils::Vector<vm::InternalCallEntry>& entries) noexcept;

    static void get_newobj_internal_call_entries(utils::Vector<vm::NewobjInternalCallEntry>& entries) noexcept;
};
} // namespace icalls
} // namespace leanclr
