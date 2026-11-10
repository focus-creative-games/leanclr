// Standalone Windows benchmark host adapter. Some Unity Editor distributions
// omit Libil2cppLackey/DllMain.cpp used by the standalone IL2CPP CLI. Preserve
// its callback contract from libil2cpp/os/Win32/DllMain.cpp: forward DLL thread
// detach notifications to IL2CPP's own ThreadImpl::OnCurrentThreadExiting.
// This file is not compiled into LeanCLR or any generated managed method.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
using DllMainFunc = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
static DllMainFunc callback;

__declspec(dllexport) void Libil2cppLackeySetDllMain(DllMainFunc dll_main)
{
    callback = dll_main;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return callback ? callback(instance, reason, reserved) : TRUE;
}
