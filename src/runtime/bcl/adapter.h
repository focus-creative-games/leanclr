#pragma once
#include "core/rt_base.h"

namespace leanclr
{
namespace metadata
{
class RtModuleDef;
}

namespace bcl
{
// Link-time adapter contract. The core owns class initialization and execution;
// the selected adapter supplies managed names, ABI validation and registrations.
class Adapter
{
  public:
    static RtResultVoid initialize_corlib(metadata::RtModuleDef* corlib);
    static RtResultVoid validate_managed_layouts();
    static void register_internal_calls();
    static void register_intrinsics();
};
} // namespace bcl
} // namespace leanclr
