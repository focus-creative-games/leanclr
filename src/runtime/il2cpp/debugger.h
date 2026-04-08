#pragma once

#include "il2cpp_api_types.h"

namespace leanclr
{
namespace il2cpp
{
class Debugger
{
  public:
  static void set_agent_options(const char* options);
  static bool is_debugger_attached();
  static void register_debugger_agent_transport(Il2CppDebuggerTransport* t);
  static bool get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* info);
};
} // namespace il2cpp
} // namespace leanclr