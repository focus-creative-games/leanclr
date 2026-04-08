#include "debugger.h"

namespace leanclr
{
namespace il2cpp
{

void Debugger::set_agent_options(const char* options)
{
    (void)options;
}

bool Debugger::is_debugger_attached()
{
    return false;
}

void Debugger::register_debugger_agent_transport(Il2CppDebuggerTransport* t)
{
    (void)t;
}

bool Debugger::get_method_info(const MethodInfo* method, Il2CppMethodDebugInfo* info)
{
    return false;
}

} // namespace il2cpp
} // namespace leanclr