#pragma once

#include "il2cpp_api_types.h"

namespace leanclr
{
namespace il2cpp
{
class UnityEngine
{
  public:
  static void set_unitytls_interface(const void* unitytlsInterfaceStruct);
  static const void* get_unitytls_interface();
  static void set_android_network_up_state_func(Il2CppAndroidUpStateFunc func);
  static Il2CppAndroidUpStateFunc get_android_network_up_state_func();
};
} // namespace il2cpp
} // namespace leanclr