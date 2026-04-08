#include "unityengine.h"

namespace leanclr
{
namespace il2cpp
{

static const void* s_unitytls_interface_struct = nullptr;
static Il2CppAndroidUpStateFunc s_android_network_up_state_func = nullptr;

void UnityEngine::set_unitytls_interface(const void* unitytlsInterfaceStruct)
{
    s_unitytls_interface_struct = unitytlsInterfaceStruct;
}

const void* UnityEngine::get_unitytls_interface()
{
    return s_unitytls_interface_struct;
}

void UnityEngine::set_android_network_up_state_func(Il2CppAndroidUpStateFunc func)
{
    s_android_network_up_state_func = func;
}

Il2CppAndroidUpStateFunc UnityEngine::get_android_network_up_state_func()
{
    return s_android_network_up_state_func;
}
} // namespace il2cpp
} // namespace leanclr