#pragma once

#include "icall_base.h"

namespace leanclr
{
namespace icalls
{

class SystemException
{
  public:
    static utils::Span<vm::InternalCallEntry> get_internal_call_entries() noexcept;
    static utils::Span<vm::InternalCallEntry> get_net10_internal_call_entries() noexcept;

    // Report an unhandled exception
    static RtResultVoid report_unhandled_exception(vm::RtException* exception) noexcept;
    static bool is_immutable_agile_exception(vm::RtException* exception) noexcept;
    static void prepare_for_foreign_exception_raise() noexcept;
    static vm::RtObject* get_frozen_stack_trace(vm::RtException* exception) noexcept;
    static uint32_t get_exception_count() noexcept;
};

} // namespace icalls
} // namespace leanclr
