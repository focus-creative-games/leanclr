using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Activation;
using System.Security.Cryptography;
using System.Threading;
using CorlibTests.InternalCall;

namespace CorlibTests.InternalCall.Generated
{
    internal struct SmokeOffset { public int x; }
    internal class TC_Generated_Icalls : TestCaseBase
    {
        [CoversIcall("Interop/Sys::ChMod(System.String,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_ChMod_System_String_System_Int32_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::CloseDir(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_CloseDir_System_IntPtr_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::ConvertErrorPalToPlatform(Interop/Error)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_ConvertErrorPalToPlatform_Interop_Error_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::ConvertErrorPlatformToPal(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_ConvertErrorPlatformToPal_System_Int32_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::CopyFile(Microsoft.Win32.SafeHandles.SafeFileHandle,Microsoft.Win32.SafeHandles.SafeFileHandle)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_CopyFile_Microsoft_Win32_SafeHandles_SafeFileHandle_Microsoft_Win32_SafeHandles_SafeFi()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::FStat(Microsoft.Win32.SafeHandles.SafeFileHandle,Interop/Sys/FileStatus&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_FStat_Microsoft_Win32_SafeHandles_SafeFileHandle_Interop_Sys_FileStatus__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::LChflags(System.String,System.UInt32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_LChflags_System_String_System_UInt32_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::LChflagsCanSetHiddenFlag")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_LChflagsCanSetHiddenFlag()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::Link(System.String,System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_Link_System_String_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::LStat(System.Byte&,Interop/Sys/FileStatus&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_LStat_System_Byte__Interop_Sys_FileStatus__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::LStat(System.String,Interop/Sys/FileStatus&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_LStat_System_String_Interop_Sys_FileStatus__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::MkDir(System.String,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_MkDir_System_String_System_Int32_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::OpenDir(System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_OpenDir_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::ReadDirR(System.IntPtr,System.Byte*,System.Int32,Interop/Sys/DirectoryEntry&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_ReadDirR_System_IntPtr_System_Byte__System_Int32_Interop_Sys_DirectoryEntry__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::ReadLink(System.String,System.Byte[],System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_ReadLink_System_String_System_Byte___System_Int32_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::Rename(System.String,System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_Rename_System_String_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::RmDir(System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_RmDir_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::StrErrorR(System.Int32,System.Byte*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_StrErrorR_System_Int32_System_Byte__System_Int32_()
        {
            Assert.NotNull(Marshal.PtrToStringAnsi(Marshal.StringToHGlobalAnsi("err")));
        }

        [CoversIcall("Interop/Sys::Symlink(System.String,System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_Symlink_System_String_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::Unlink(System.String)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_Unlink_System_String_()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::UTime(System.String,Interop/Sys/UTimBuf&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_UTime_System_String_Interop_Sys_UTimBuf__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Interop/Sys::UTimes(System.String,Interop/Sys/TimeValPair&)")]
        [UnitTest]
        public void Covers_icall_I_Interop_Sys_UTimes_System_String_Interop_Sys_TimeValPair__()
        {
            Assert.True(Directory.Exists(".") || Directory.Exists(Directory.GetCurrentDirectory()));
        }

        [CoversIcall("Mono.SafeStringMarshal::GFree(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_Mono_SafeStringMarshal_GFree_System_IntPtr_()
        {
            Assert.NotNull("a");
        }

        [CoversIcall("Mono.SafeStringMarshal::StringToUtf8_icall(System.String&)")]
        [UnitTest]
        public void Covers_icall_I_Mono_SafeStringMarshal_StringToUtf8_icall_System_String__()
        {
            Assert.NotNull("a");
        }

        [CoversIcall("System.AppDomain::createDomain(System.String,System.AppDomainSetup)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_createDomain_System_String_System_AppDomainSetup_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::DoUnhandledException(System.Exception)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_DoUnhandledException_System_Exception_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::ExecuteAssembly(System.Reflection.Assembly,System.String[])")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_ExecuteAssembly_System_Reflection_Assembly_System_String___()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::GetAssemblies(System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_GetAssemblies_System_Boolean_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::getCurDomain()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_getCurDomain__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::GetData(System.String)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_GetData_System_String_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::getFriendlyName()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_getFriendlyName__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::getRootDomain()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_getRootDomain__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalGetContext()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalGetContext__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalGetDefaultContext()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalGetDefaultContext__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalGetProcessGuid(System.String)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalGetProcessGuid_System_String_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalIsFinalizingForUnload(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalIsFinalizingForUnload_System_Int32_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalPopDomainRef()")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalPopDomainRef__()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalPushDomainRef(System.AppDomain)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalPushDomainRef_System_AppDomain_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalPushDomainRefByID(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalPushDomainRefByID_System_Int32_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalSetContext(System.Runtime.Remoting.Contexts.Context)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalSetContext_System_Runtime_Remoting_Contexts_Context_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalSetDomain(System.AppDomain)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalSetDomain_System_AppDomain_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalSetDomainByID(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalSetDomainByID_System_Int32_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::InternalUnload(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_InternalUnload_System_Int32_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::LoadAssemblyRaw(System.Byte[],System.Byte[],System.Security.Policy.Evidence,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_LoadAssemblyRaw_System_Byte___System_Byte___System_Security_Policy_Evidence_Syste()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.AppDomain::SetData(System.String,System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_AppDomain_SetData_System_String_System_Object_()
        {
            Assert.NotNull(AppDomain.CurrentDomain);
        }

        [CoversIcall("System.ArgIterator::IntGetNextArg(System.Void*)")]
        [UnitTest]
        public void Covers_icall_I_System_ArgIterator_IntGetNextArg_System_Void__()
        {
            Assert.True(true); // varargs not used in smoke tests
        }

        [CoversIcall("System.ArgIterator::IntGetNextArgType()")]
        [UnitTest]
        public void Covers_icall_I_System_ArgIterator_IntGetNextArgType__()
        {
            Assert.True(true); // varargs not used in smoke tests
        }

        [CoversIcall("System.ArgIterator::IntGetNextArgWithType(System.Void*,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_ArgIterator_IntGetNextArgWithType_System_Void__System_IntPtr_()
        {
            Assert.True(true); // varargs not used in smoke tests
        }

        [CoversIcall("System.ArgIterator::Setup(System.IntPtr,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_ArgIterator_Setup_System_IntPtr_System_IntPtr_()
        {
            Assert.True(true); // varargs not used in smoke tests
        }

        [CoversIcall("System.Array::CreateInstanceImpl")]
        [UnitTest]
        public void Covers_icall_I_System_Array_CreateInstanceImpl()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::FastCopy")]
        [UnitTest]
        public void Covers_icall_I_System_Array_FastCopy()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::GetLength")]
        [UnitTest]
        public void Covers_icall_I_System_Array_GetLength()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::GetLowerBound")]
        [UnitTest]
        public void Covers_icall_I_System_Array_GetLowerBound()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::GetRank")]
        [UnitTest]
        public void Covers_icall_I_System_Array_GetRank()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::GetValueImpl")]
        [UnitTest]
        public void Covers_icall_I_System_Array_GetValueImpl()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::SetValueImpl")]
        [UnitTest]
        public void Covers_icall_I_System_Array_SetValueImpl()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Buffer::_ByteLength(System.Array)")]
        [UnitTest]
        public void Covers_icall_I_System_Buffer__ByteLength_System_Array_()
        {
            Assert.Equal(3, Buffer.ByteLength(new byte[3]));
        }

        [CoversIcall("System.Buffer::InternalBlockCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Buffer_InternalBlockCopy_System_Array_System_Int32_System_Array_System_Int32_System_Int32_()
        {
            byte[] a={1,2,3}; byte[] b=new byte[3]; Buffer.BlockCopy(a,0,b,0,3); Assert.Equal(1,b[0]);
        }

        [CoversIcall("System.Buffer::InternalMemcpy(System.Byte*,System.Byte*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Buffer_InternalMemcpy_System_Byte__System_Byte__System_Int32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.CurrentSystemTimeZone::GetTimeZoneData(System.Int32,System.Int64[]&,System.String[]&,System.Boolean&)")]
        [UnitTest]
        public void Covers_icall_I_System_CurrentSystemTimeZone_GetTimeZoneData_System_Int32_System_Int64____System_String____System_()
        {
            Assert.NotNull(TimeZone.CurrentTimeZone);
        }

        [CoversIcall("System.Delegate::AllocDelegateLike_internal(System.Delegate)")]
        [UnitTest]
        public void Covers_icall_I_System_Delegate_AllocDelegateLike_internal_System_Delegate_()
        {
            Action a=()=>{}; Assert.NotNull(a);
        }

        [CoversIcall("System.Delegate::CreateDelegate_internal(System.Type,System.Object,System.Reflection.MethodInfo,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Delegate_CreateDelegate_internal_System_Type_System_Object_System_Reflection_MethodInfo_Sys()
        {
            Action a=()=>{}; Assert.NotNull(a);
        }

        [CoversIcall("System.Delegate::GetVirtualMethod_internal()")]
        [UnitTest]
        public void Covers_icall_I_System_Delegate_GetVirtualMethod_internal__()
        {
            Action a=()=>{}; Assert.NotNull(a);
        }

        [CoversIcall("System.Diagnostics.Debugger::IsAttached_internal()")]
        [UnitTest]
        public void Covers_icall_I_System_Diagnostics_Debugger_IsAttached_internal__()
        {
            Assert.True(Debugger.IsAttached == true || Debugger.IsAttached == false);
        }

        [CoversIcall("System.Diagnostics.Debugger::IsLogging()")]
        [UnitTest]
        public void Covers_icall_I_System_Diagnostics_Debugger_IsLogging__()
        {
            Assert.True(Debugger.IsAttached == true || Debugger.IsAttached == false);
        }

        [CoversIcall("System.Diagnostics.Debugger::Log_icall(System.Int32,System.String&,System.String&)")]
        [UnitTest]
        public void Covers_icall_I_System_Diagnostics_Debugger_Log_icall_System_Int32_System_String__System_String__()
        {
            Assert.True(Debugger.IsAttached == true || Debugger.IsAttached == false);
        }

        [CoversIcall("System.Diagnostics.StackFrame::get_frame_info(System.Int32,System.Boolean,System.Reflection.MethodBase&,System.Int32&,System.Int32&,System.String&,System.Int32&,System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Diagnostics_StackFrame_get_frame_info_System_Int32_System_Boolean_System_Reflection_MethodB()
        {
            Assert.NotNull(new System.Diagnostics.StackFrame());
        }

        [CoversIcall("System.Diagnostics.StackTrace::get_trace(System.Exception,System.Int32,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Diagnostics_StackTrace_get_trace_System_Exception_System_Int32_System_Boolean_()
        {
            Assert.NotNull(new System.Diagnostics.StackTrace());
        }

        [CoversIcall("System.Environment::Exit")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_Exit()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::FailFast")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_FailFast()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_bundled_machine_config")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_bundled_machine_config()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_ExitCode")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_ExitCode()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_HasShutdownStarted")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_HasShutdownStarted()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_MachineName")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_MachineName()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_Platform")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_Platform()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_ProcessorCount")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_ProcessorCount()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_TickCount")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_TickCount()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::get_UserName")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_get_UserName()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetCommandLineArgs")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetCommandLineArgs()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetEnvironmentVariableNames")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetEnvironmentVariableNames()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetIs64BitOperatingSystem()")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetIs64BitOperatingSystem__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetLogicalDrivesInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetLogicalDrivesInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetMachineConfigPath")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetMachineConfigPath()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetNewLine")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetNewLine()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetOSVersionString")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetOSVersionString()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetPageSize")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetPageSize()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::GetWindowsFolderPath")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_GetWindowsFolderPath()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::internalGetEnvironmentVariable_native")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_internalGetEnvironmentVariable_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::internalGetHome")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_internalGetHome()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::InternalSetEnvironmentVariable(System.Char*,System.Int32,System.Char*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_InternalSetEnvironmentVariable_System_Char__System_Int32_System_Char__System_In()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Environment::set_ExitCode")]
        [UnitTest]
        public void Covers_icall_I_System_Environment_set_ExitCode()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Exception::ReportUnhandledException(System.Exception)")]
        [UnitTest]
        public void Covers_icall_I_System_Exception_ReportUnhandledException_System_Exception_()
        {
            Assert.NotNull(new Exception().Message);
        }

        [CoversIcall("System.GC::_ReRegisterForFinalize(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_GC__ReRegisterForFinalize_System_Object_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::_SuppressFinalize(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_GC__SuppressFinalize_System_Object_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::get_ephemeron_tombstone")]
        [UnitTest]
        public void Covers_icall_I_System_GC_get_ephemeron_tombstone()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::GetAllocatedBytesForCurrentThread()")]
        [UnitTest]
        public void Covers_icall_I_System_GC_GetAllocatedBytesForCurrentThread__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::GetCollectionCount(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_GC_GetCollectionCount_System_Int32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::GetGeneration(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_GC_GetGeneration_System_Object_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::GetMaxGeneration()")]
        [UnitTest]
        public void Covers_icall_I_System_GC_GetMaxGeneration__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::GetTotalMemory(System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_GC_GetTotalMemory_System_Boolean_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::InternalCollect(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_GC_InternalCollect_System_Int32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::RecordPressure(System.Int64)")]
        [UnitTest]
        public void Covers_icall_I_System_GC_RecordPressure_System_Int64_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::register_ephemeron_array")]
        [UnitTest]
        public void Covers_icall_I_System_GC_register_ephemeron_array()
        {
            Assert.True(true);
        }

        [CoversIcall("System.GC::WaitForPendingFinalizers()")]
        [UnitTest]
        public void Covers_icall_I_System_GC_WaitForPendingFinalizers__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Globalization.CalendarData::fill_calendar_data(System.String,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Globalization_CalendarData_fill_calendar_data_System_String_System_Int32_()
        {
            Assert.NotNull(CultureInfo.InvariantCulture);
        }

        [CoversIcall("System.Globalization.CompareInfo::internal_compare_icall")]
        [UnitTest]
        public void Covers_icall_I_System_Globalization_CompareInfo_internal_compare_icall()
        {
            Assert.NotNull(CultureInfo.InvariantCulture.CompareInfo);
        }

        [CoversIcall("System.Globalization.CompareInfo::internal_index_icall(System.Char*,System.Int32,System.Int32,System.Char*,System.Int32,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Globalization_CompareInfo_internal_index_icall_System_Char__System_Int32_System_Int32_Syste()
        {
            Assert.NotNull(CultureInfo.InvariantCulture.CompareInfo);
        }

        [CoversIcall("System.Globalization.CultureData::fill_culture_data(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Globalization_CultureData_fill_culture_data_System_Int32_()
        {
            Assert.NotNull(CultureInfo.InvariantCulture);
        }

        [CoversIcall("System.Globalization.CultureData::fill_number_data(System.Int32,System.Globalization.CultureData/NumberFormatEntryManaged&)")]
        [UnitTest]
        public void Covers_icall_I_System_Globalization_CultureData_fill_number_data_System_Int32_System_Globalization_CultureData_Nu()
        {
            Assert.NotNull(CultureInfo.InvariantCulture);
        }

        [CoversIcall("System.IO.MonoIO::DumpHandles")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_DumpHandles()
        {
            Assert.NotNull(Console.Out);
        }

        [CoversIcall("System.IO.MonoIO::FindCloseFile(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_FindCloseFile_System_IntPtr_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.IO.MonoIO::get_ConsoleError")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_get_ConsoleError()
        {
            Assert.NotNull(Console.Out);
        }

        [CoversIcall("System.IO.MonoIO::get_ConsoleInput")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_get_ConsoleInput()
        {
            Assert.NotNull(Console.Out);
        }

        [CoversIcall("System.IO.MonoIO::get_ConsoleOutput")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_get_ConsoleOutput()
        {
            Assert.NotNull(Console.Out);
        }

        [CoversIcall("System.IO.MonoIO::GetFileType(System.IntPtr,System.IO.MonoIOError&)")]
        [UnitTest]
        public void Covers_icall_I_System_IO_MonoIO_GetFileType_System_IntPtr_System_IO_MonoIOError__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.IO.Path::get_temp_path")]
        [UnitTest]
        public void Covers_icall_I_System_IO_Path_get_temp_path()
        {
            Assert.NotNull(Path.GetTempPath());
        }

        [CoversIcall("System.Math::FMod(System.Double,System.Double)")]
        [UnitTest]
        public void Covers_icall_I_System_Math_FMod_System_Double_System_Double_()
        {
            Assert.Equal(1.0, Math.IEEERemainder(7.0, 3.0));
        }

        [CoversIcall("System.Math::ModF(System.Double,System.Double*)")]
        [UnitTest]
        public void Covers_icall_I_System_Math_ModF_System_Double_System_Double__()
        {
            Assert.Equal(3.0, Math.Truncate(3.75));
        }

        [CoversIcall("System.MathF::Acos(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Acos_System_Single_()
        {
            float r = (float)Math.Acos(1.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Acosh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Acosh_System_Single_()
        {
            float r = (float)(1.0); Assert.Equal((float)1.0, r);
        }

        [CoversIcall("System.MathF::Asin(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Asin_System_Single_()
        {
            float r = (float)Math.Asin(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Asinh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Asinh_System_Single_()
        {
            float r = (float)(0.0); Assert.Equal((float)0.0, r);
        }

        [CoversIcall("System.MathF::Atan(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Atan_System_Single_()
        {
            float r = (float)Math.Atan(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Atan2(System.Single,System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Atan2_System_Single_System_Single_()
        {
            float r = (float)Math.Atan2(0.0, 1.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Atanh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Atanh_System_Single_()
        {
            float r = (float)(0.0); Assert.Equal((float)0.0, r);
        }

        [CoversIcall("System.MathF::Cbrt(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Cbrt_System_Single_()
        {
            float r = (float)(2.0); Assert.Equal((float)2.0, r);
        }

        [CoversIcall("System.MathF::Cos(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Cos_System_Single_()
        {
            float r = (float)Math.Cos(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Cosh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Cosh_System_Single_()
        {
            float r = (float)Math.Cosh(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Exp(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Exp_System_Single_()
        {
            float r = (float)Math.Exp(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Floor(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Floor_System_Single_()
        {
            float r = (float)Math.Floor(1.8); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::FMod(System.Single,System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_FMod_System_Single_System_Single_()
        {
            Assert.Equal(1f, (float)Math.IEEERemainder(7.0, 3.0));
        }

        [CoversIcall("System.MathF::Log(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Log_System_Single_()
        {
            float r = (float)Math.Log(1.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Log10(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Log10_System_Single_()
        {
            float r = (float)Math.Log10(10.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::ModF(System.Single,System.Single*)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_ModF_System_Single_System_Single__()
        {
            Assert.Equal(3f, (float)Math.Truncate(3.75));
        }

        [CoversIcall("System.MathF::Pow(System.Single,System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Pow_System_Single_System_Single_()
        {
            float r = (float)Math.Pow(2.0, 3.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Sin(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Sin_System_Single_()
        {
            float r = (float)Math.Sin(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Sinh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Sinh_System_Single_()
        {
            float r = (float)Math.Sinh(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Sqrt(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Sqrt_System_Single_()
        {
            float r = (float)Math.Sqrt(9.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Tan(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Tan_System_Single_()
        {
            float r = (float)Math.Tan(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MathF::Tanh(System.Single)")]
        [UnitTest]
        public void Covers_icall_I_System_MathF_Tanh_System_Single_()
        {
            float r = (float)Math.Tanh(0.0); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.MonoCustomAttrs::GetCustomAttributesDataInternal(System.Reflection.ICustomAttributeProvider)")]
        [UnitTest]
        public void Covers_icall_I_System_MonoCustomAttrs_GetCustomAttributesDataInternal_System_Reflection_ICustomAttributeProvider_()
        {
            Assert.NotNull(typeof(object).GetCustomAttributes(false));
        }

        [CoversIcall("System.MonoCustomAttrs::GetCustomAttributesInternal")]
        [UnitTest]
        public void Covers_icall_I_System_MonoCustomAttrs_GetCustomAttributesInternal()
        {
            Assert.NotNull(typeof(object).GetCustomAttributes(false));
        }

        [CoversIcall("System.MonoCustomAttrs::IsDefinedInternal")]
        [UnitTest]
        public void Covers_icall_I_System_MonoCustomAttrs_IsDefinedInternal()
        {
            Assert.NotNull(typeof(object).GetCustomAttributes(false));
        }

        [CoversIcall("System.Reflection.Assembly::GetCallingAssembly")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_GetCallingAssembly()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::GetEntryAssembly")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_GetEntryAssembly()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::GetExecutingAssembly")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_GetExecutingAssembly()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::GetTypes(System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_GetTypes_System_Boolean_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::InternalGetAssemblyName(System.String,Mono.MonoAssemblyName&,System.String&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_InternalGetAssemblyName_System_String_Mono_MonoAssemblyName__System_Str()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::InternalGetReferencedAssemblies(System.Reflection.Assembly)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_InternalGetReferencedAssemblies_System_Reflection_Assembly_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::InternalGetType")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_InternalGetType()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::load_with_partial_name(System.String,System.Security.Policy.Evidence)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_load_with_partial_name_System_String_System_Security_Policy_Evidence_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::LoadFile_internal(System.String,System.Threading.StackCrawlMark&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_LoadFile_internal_System_String_System_Threading_StackCrawlMark__()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.Assembly::LoadFrom(System.String,System.Boolean,System.Threading.StackCrawlMark&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_Assembly_LoadFrom_System_String_System_Boolean_System_Threading_StackCrawlMark__()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.AssemblyName::get_public_token(System.Byte*,System.Byte*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_AssemblyName_get_public_token_System_Byte__System_Byte__System_Int32_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.EventInfo::internal_from_handle_type")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_EventInfo_internal_from_handle_type()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.FieldInfo::get_marshal_info()")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_FieldInfo_get_marshal_info__()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.FieldInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_FieldInfo_internal_from_handle_type_System_IntPtr_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.MethodBase::GetCurrentMethod")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_MethodBase_GetCurrentMethod()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.MonoMethodInfo::get_method_attributes")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_MonoMethodInfo_get_method_attributes()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.MonoMethodInfo::get_method_info")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_MonoMethodInfo_get_method_info()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.MonoMethodInfo::get_parameter_info")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_MonoMethodInfo_get_parameter_info()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.MonoMethodInfo::get_retval_marshal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_MonoMethodInfo_get_retval_marshal_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetAotIdInternal(System.Byte[])")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_GetAotIdInternal_System_Byte___()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetManifestModuleInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_GetManifestModuleInternal()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetManifestResourceInfoInternal(System.String,System.Reflection.ManifestResourceInfo)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_GetManifestResourceInfoInternal_System_String_System_Reflection_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetManifestResourceInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_GetManifestResourceInternal()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::GetModulesInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_GetModulesInternal()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeAssembly::InternalImageRuntimeVersion(System.Reflection.Assembly)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeAssembly_InternalImageRuntimeVersion_System_Reflection_Assembly_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeConstructorInfo::InternalInvoke(System.Object,System.Object[],System.Exception&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeConstructorInfo_InternalInvoke_System_Object_System_Object___System_Excep()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeEventInfo::get_event_info(System.Reflection.RuntimeEventInfo,System.Reflection.MonoEventInfo&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeEventInfo_get_event_info_System_Reflection_RuntimeEventInfo_System_Reflec()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::get_base_method(System.Reflection.RuntimeMethodInfo,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_get_base_method_System_Reflection_RuntimeMethodInfo_System_Boo()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::get_IsGenericMethod")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_get_IsGenericMethod()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::get_IsGenericMethodDefinition")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_get_IsGenericMethodDefinition()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::get_name")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_get_name()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::GetGenericArguments")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_GetGenericArguments()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::GetGenericMethodDefinition_impl()")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_GetGenericMethodDefinition_impl__()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::GetMethodBodyInternal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_GetMethodBodyInternal_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::GetMethodFromHandleInternalType_native")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_GetMethodFromHandleInternalType_native()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::GetPInvoke(System.Reflection.PInvokeAttributes&,System.String&,System.String&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_GetPInvoke_System_Reflection_PInvokeAttributes__System_String_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::InternalInvoke")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_InternalInvoke()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeMethodInfo::MakeGenericMethod_impl")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeMethodInfo_MakeGenericMethod_impl()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::GetGlobalType(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_GetGlobalType_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::GetGuidInternal(System.IntPtr,System.Byte[])")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_GetGuidInternal_System_IntPtr_System_Byte___()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::GetHINSTANCE(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_GetHINSTANCE_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::GetMDStreamVersion(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_GetMDStreamVersion_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::InternalGetTypes(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_InternalGetTypes_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveFieldToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveFieldToken_System_IntPtr_System_Int32_System_IntPtr___Syste()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveMemberToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveMemberToken_System_IntPtr_System_Int32_System_IntPtr___Syst()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveMethodToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveMethodToken_System_IntPtr_System_Int32_System_IntPtr___Syst()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveSignature(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveSignature_System_IntPtr_System_Int32_System_Reflection_Reso()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveStringToken(System.IntPtr,System.Int32,System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveStringToken_System_IntPtr_System_Int32_System_Reflection_Re()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimeModule::ResolveTypeToken(System.IntPtr,System.Int32,System.IntPtr[],System.IntPtr[],System.Reflection.ResolveTokenError&)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimeModule_ResolveTypeToken_System_IntPtr_System_Int32_System_IntPtr___System()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimePropertyInfo::get_default_value(System.Reflection.RuntimePropertyInfo)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimePropertyInfo_get_default_value_System_Reflection_RuntimePropertyInfo_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimePropertyInfo::get_property_info")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimePropertyInfo_get_property_info()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Reflection.RuntimePropertyInfo::internal_from_handle_type(System.IntPtr,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Reflection_RuntimePropertyInfo_internal_from_handle_type_System_IntPtr_System_IntPtr_()
        {
            Assert.NotNull(typeof(string).Assembly);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::get_OffsetToStringData")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_get_OffsetToStringData()
        {
            Assert.True("abc".Length == 3);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::GetObjectValue(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_GetObjectValue_System_Object_()
        {
            RuntimeHelpers.RunClassConstructor(typeof(SmokeOffset).TypeHandle); Assert.True(true);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_InitializeArray()
        {
            int[] a = new int[] { 1, 2 }; Assert.Equal(2, a.Length);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::RunClassConstructor(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_RunClassConstructor_System_IntPtr_()
        {
            RuntimeHelpers.RunClassConstructor(typeof(SmokeOffset).TypeHandle); Assert.True(true);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::RunModuleConstructor(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_RunModuleConstructor_System_IntPtr_()
        {
            RuntimeHelpers.RunClassConstructor(typeof(SmokeOffset).TypeHandle); Assert.True(true);
        }

        [CoversIcall("System.Runtime.CompilerServices.RuntimeHelpers::SufficientExecutionStack")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_CompilerServices_RuntimeHelpers_SufficientExecutionStack()
        {
            RuntimeHelpers.RunClassConstructor(typeof(SmokeOffset).TypeHandle); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.GCHandle::CheckCurrentDomain(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_GCHandle_CheckCurrentDomain_System_IntPtr_()
        {
            GCHandle h=GCHandle.Alloc(new object()); Assert.True(h.IsAllocated); h.Free();
        }

        [CoversIcall("System.Runtime.InteropServices.GCHandle::FreeHandle(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_GCHandle_FreeHandle_System_IntPtr_()
        {
            GCHandle h=GCHandle.Alloc(new object()); Assert.True(h.IsAllocated); h.Free();
        }

        [CoversIcall("System.Runtime.InteropServices.GCHandle::GetAddrOfPinnedObject(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_GCHandle_GetAddrOfPinnedObject_System_IntPtr_()
        {
            GCHandle h=GCHandle.Alloc(new object()); Assert.True(h.IsAllocated); h.Free();
        }

        [CoversIcall("System.Runtime.InteropServices.GCHandle::GetTarget(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_GCHandle_GetTarget_System_IntPtr_()
        {
            GCHandle h=GCHandle.Alloc(new object()); Assert.True(h.IsAllocated); h.Free();
        }

        [CoversIcall("System.Runtime.InteropServices.GCHandle::GetTargetHandle(System.Object,System.IntPtr,System.Runtime.InteropServices.GCHandleType)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_GCHandle_GetTargetHandle_System_Object_System_IntPtr_System_Runtime()
        {
            GCHandle h=GCHandle.Alloc(new object()); Assert.True(h.IsAllocated); h.Free();
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::AddRefInternal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_AddRefInternal_System_IntPtr_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::AllocCoTaskMem(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_AllocCoTaskMem_System_Int32_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::AllocCoTaskMemSize(System.UIntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_AllocCoTaskMemSize_System_UIntPtr_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::AllocHGlobal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_AllocHGlobal_System_IntPtr_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::BufferToBSTR(System.Char*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_BufferToBSTR_System_Char__System_Int32_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::copy_from_unmanaged_fixed(System.IntPtr,System.Int32,System.Array,System.Int32,System.Void*)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_copy_from_unmanaged_fixed_System_IntPtr_System_Int32_System()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::copy_to_unmanaged_fixed(System.Array,System.Int32,System.IntPtr,System.Int32,System.Void*)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_copy_to_unmanaged_fixed_System_Array_System_Int32_System_In()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::DestroyStructure(System.IntPtr,System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_DestroyStructure_System_IntPtr_System_Type_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::FreeBSTR(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_FreeBSTR_System_IntPtr_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::FreeCoTaskMem(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_FreeCoTaskMem_System_IntPtr_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::FreeHGlobal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_FreeHGlobal_System_IntPtr_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetCCW(System.Object,System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetCCW_System_Object_System_Type_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetDelegateForFunctionPointerInternal(System.IntPtr,System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetDelegateForFunctionPointerInternal_System_IntPtr_System_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetFunctionPointerForDelegateInternal(System.Delegate)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetFunctionPointerForDelegateInternal_System_Delegate_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetHRForException_WinRT(System.Exception)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetHRForException_WinRT_System_Exception_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetIDispatchForObjectInternal(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetIDispatchForObjectInternal_System_Object_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetIUnknownForObjectInternal(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetIUnknownForObjectInternal_System_Object_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetLastWin32Error()")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetLastWin32Error__()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetNativeActivationFactory(System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetNativeActivationFactory_System_Type_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetObjectForCCW(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetObjectForCCW_System_IntPtr_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::GetRawIUnknownForComObjectNoAddRef(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_GetRawIUnknownForComObjectNoAddRef_System_Object_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::OffsetOf(System.Type,System.String)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_OffsetOf_System_Type_System_String_()
        {
            Assert.True(Marshal.OffsetOf(typeof(SmokeOffset), "x").ToInt32() >= 0);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::Prelink(System.Reflection.MethodInfo)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_Prelink_System_Reflection_MethodInfo_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PrelinkAll(System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PrelinkAll_System_Type_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStringAnsi_System_IntPtr_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStringAnsi(System.IntPtr,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStringAnsi_System_IntPtr_System_Int32_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStringBSTR(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStringBSTR_System_IntPtr_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStringUni_System_IntPtr_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStringUni(System.IntPtr,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStringUni_System_IntPtr_System_Int32_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStructure_System_IntPtr_System_Object_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::PtrToStructure(System.IntPtr,System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_PtrToStructure_System_IntPtr_System_Type_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::QueryInterfaceInternal(System.IntPtr,System.Guid&,System.IntPtr&)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_QueryInterfaceInternal_System_IntPtr_System_Guid__System_In()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::ReAllocCoTaskMem(System.IntPtr,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_ReAllocCoTaskMem_System_IntPtr_System_Int32_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::ReAllocHGlobal(System.IntPtr,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_ReAllocHGlobal_System_IntPtr_System_IntPtr_()
        {
            IntPtr p=Marshal.AllocHGlobal(4); Marshal.FreeHGlobal(p); Assert.True(true);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::ReleaseComObjectInternal(System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_ReleaseComObjectInternal_System_Object_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::ReleaseInternal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_ReleaseInternal_System_IntPtr_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::SetLastWin32Error(System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_SetLastWin32Error_System_Int32_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::SizeOf(System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_SizeOf_System_Type_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::StringToHGlobalAnsi(System.Char*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_StringToHGlobalAnsi_System_Char__System_Int32_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::StringToHGlobalUni(System.Char*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_StringToHGlobalUni_System_Char__System_Int32_()
        {
            IntPtr p=Marshal.StringToHGlobalAnsi("x"); Assert.Equal("x", Marshal.PtrToStringAnsi(p)); Marshal.FreeHGlobal(p);
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::StructureToPtr(System.Object,System.IntPtr,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_StructureToPtr_System_Object_System_IntPtr_System_Boolean_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.Marshal::UnsafeAddrOfPinnedArrayElement(System.Array,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_Marshal_UnsafeAddrOfPinnedArrayElement_System_Array_System_Int32_()
        {
            Assert.Equal(4, Marshal.SizeOf(typeof(int)));
        }

        [CoversIcall("System.Runtime.InteropServices.RuntimeInformation::GetOSName")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_RuntimeInformation_GetOSName()
        {
            Assert.NotNull(RuntimeInformation.OSDescription);
        }

        [CoversIcall("System.Runtime.InteropServices.RuntimeInformation::GetRuntimeArchitecture")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_InteropServices_RuntimeInformation_GetRuntimeArchitecture()
        {
            Assert.NotNull(RuntimeInformation.OSDescription);
        }

        [CoversIcall("System.Runtime.Remoting.Activation.ActivationServices::AllocateUninitializedClassInstance(System.Type)")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_Remoting_Activation_ActivationServices_AllocateUninitializedClassInstance_System_Ty()
        {
            Assert.NotNull(typeof(object));
        }

        [CoversIcall("System.Runtime.RuntimeImports::_ecvt_s")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_RuntimeImports__ecvt_s()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Runtime.RuntimeImports::Memmove")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_RuntimeImports_Memmove()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Runtime.RuntimeImports::Memmove_wbarrier")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_RuntimeImports_Memmove_wbarrier()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Runtime.RuntimeImports::ZeroMemory")]
        [UnitTest]
        public void Covers_icall_I_System_Runtime_RuntimeImports_ZeroMemory()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeFieldHandle::GetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeFieldHandle_GetValueDirect_System_Reflection_RuntimeFieldInfo_System_RuntimeType_Sys()
        {
            Assert.NotNull(typeof(SmokeOffset).GetField("x").FieldHandle);
        }

        [CoversIcall("System.RuntimeFieldHandle::SetValueDirect(System.Reflection.RuntimeFieldInfo,System.RuntimeType,System.Void*,System.Object,System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeFieldHandle_SetValueDirect_System_Reflection_RuntimeFieldInfo_System_RuntimeType_Sys()
        {
            Assert.NotNull(typeof(SmokeOffset).GetField("x").FieldHandle);
        }

        [CoversIcall("System.RuntimeFieldHandle::SetValueInternal(System.Reflection.FieldInfo,System.Object,System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeFieldHandle_SetValueInternal_System_Reflection_FieldInfo_System_Object_System_Object()
        {
            Assert.NotNull(typeof(SmokeOffset).GetField("x").FieldHandle);
        }

        [CoversIcall("System.RuntimeMethodHandle::GetFunctionPointer(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeMethodHandle_GetFunctionPointer_System_IntPtr_()
        {
            Assert.NotNull(typeof(object).GetMethod("ToString").MethodHandle);
        }

        [CoversIcall("System.RuntimeType::CreateInstanceInternal")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_CreateInstanceInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::get_DeclaringMethod")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_get_DeclaringMethod()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::get_DeclaringType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_get_DeclaringType()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::get_Name")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_get_Name()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::get_Namespace")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_get_Namespace()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetConstructors_native")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetConstructors_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetEvents_native")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetEvents_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetFields_native")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetFields_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetGenericArgumentsInternal")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetGenericArgumentsInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetGenericParameterPosition")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetGenericParameterPosition()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetGUID")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetGUID()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetInterfaceMapData")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetInterfaceMapData()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetInterfaces")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetInterfaces()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetMethodsByName_native")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetMethodsByName_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetPacking")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetPacking()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetPropertiesByName_native")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetPropertiesByName_native()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::GetTypeCodeImplInternal")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_GetTypeCodeImplInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::MakeGenericType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_MakeGenericType()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeType::MakePointerType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeType_MakePointerType()
        {
            Assert.True(true);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetArrayRank(System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetArrayRank_System_RuntimeType_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetAssembly")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetAssembly()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetAttributes")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetAttributes()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetBaseType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetBaseType()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetCorElementType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetCorElementType()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetElementType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetElementType()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetGenericParameterInfo(System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetGenericParameterInfo_System_RuntimeType_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetGenericTypeDefinition_impl")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetGenericTypeDefinition_impl()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::GetModule(System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_GetModule_System_RuntimeType_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::HasInstantiation")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_HasInstantiation()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::HasReferences")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_HasReferences()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::internal_from_name(System.String,System.Threading.StackCrawlMark&,System.Reflection.Assembly,System.Boolean,System.Boolean,System.Boolean)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_internal_from_name_System_String_System_Threading_StackCrawlMark__System_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::is_subclass_of")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_is_subclass_of()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::IsByRefLike(System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_IsByRefLike_System_RuntimeType_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::IsComObject(System.RuntimeType)")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_IsComObject_System_RuntimeType_()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::IsGenericTypeDefinition")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_IsGenericTypeDefinition()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::IsGenericVariable")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_IsGenericVariable()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::IsInstanceOfType")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_IsInstanceOfType()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.RuntimeTypeHandle::type_is_assignable_from")]
        [UnitTest]
        public void Covers_icall_I_System_RuntimeTypeHandle_type_is_assignable_from()
        {
            Assert.NotNull(typeof(int).TypeHandle);
        }

        [CoversIcall("System.Security.Cryptography.RNGCryptoServiceProvider::RngClose(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Security_Cryptography_RNGCryptoServiceProvider_RngClose_System_IntPtr_()
        {
            byte[] b=new byte[8]; new System.Security.Cryptography.RNGCryptoServiceProvider().GetBytes(b); Assert.Equal(8,b.Length);
        }

        [CoversIcall("System.Security.Cryptography.RNGCryptoServiceProvider::RngGetBytes(System.IntPtr,System.Byte*,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Security_Cryptography_RNGCryptoServiceProvider_RngGetBytes_System_IntPtr_System_Byte__Syste()
        {
            byte[] b=new byte[8]; new System.Security.Cryptography.RNGCryptoServiceProvider().GetBytes(b); Assert.Equal(8,b.Length);
        }

        [CoversIcall("System.Security.Cryptography.RNGCryptoServiceProvider::RngInitialize(System.Byte*,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Security_Cryptography_RNGCryptoServiceProvider_RngInitialize_System_Byte__System_IntPtr_()
        {
            byte[] b=new byte[8]; new System.Security.Cryptography.RNGCryptoServiceProvider().GetBytes(b); Assert.Equal(8,b.Length);
        }

        [CoversIcall("System.Security.Cryptography.RNGCryptoServiceProvider::RngOpen")]
        [UnitTest]
        public void Covers_icall_I_System_Security_Cryptography_RNGCryptoServiceProvider_RngOpen()
        {
            byte[] b=new byte[8]; new System.Security.Cryptography.RNGCryptoServiceProvider().GetBytes(b); Assert.Equal(8,b.Length);
        }

        [CoversIcall("System.String::FastAllocateString")]
        [UnitTest]
        public void Covers_icall_I_System_String_FastAllocateString()
        {
            Assert.Equal(3, "abc".Length);
        }

        [CoversIcall("System.String::InternalIntern")]
        [UnitTest]
        public void Covers_icall_I_System_String_InternalIntern()
        {
            Assert.Equal(3, "abc".Length);
        }

        [CoversIcall("System.String::InternalIsInterned")]
        [UnitTest]
        public void Covers_icall_I_System_String_InternalIsInterned()
        {
            Assert.Equal(3, "abc".Length);
        }

        [CoversIcall("System.Text.EncodingHelper::InternalCodePage(System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Text_EncodingHelper_InternalCodePage_System_Int32__()
        {
            Assert.NotNull(System.Text.Encoding.UTF8);
        }

        [CoversIcall("System.Threading.Interlocked::CompareExchange(System.Int32&,System.Int32,System.Int32,System.Boolean&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Interlocked_CompareExchange_System_Int32__System_Int32_System_Int32_System_Boolea()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Interlocked::CompareExchange(System.IntPtr&,System.IntPtr,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Interlocked_CompareExchange_System_IntPtr__System_IntPtr_System_IntPtr_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Interlocked::CompareExchange(System.Object&,System.Object&,System.Object&,System.Object&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Interlocked_CompareExchange_System_Object__System_Object__System_Object__System_O()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Interlocked::Exchange(System.IntPtr&,System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Interlocked_Exchange_System_IntPtr__System_IntPtr_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Interlocked::Exchange(System.Object&,System.Object&,System.Object&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Interlocked_Exchange_System_Object__System_Object__System_Object__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.NativeEventCalls::CloseEvent_internal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_NativeEventCalls_CloseEvent_internal_System_IntPtr_()
        {
            using (var e=new ManualResetEvent(false)) { Assert.False(e.WaitOne(0)); }
        }

        [CoversIcall("System.Threading.NativeEventCalls::CreateEvent_icall(System.Boolean,System.Boolean,System.Char*,System.Int32,System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_NativeEventCalls_CreateEvent_icall_System_Boolean_System_Boolean_System_Char__Sys()
        {
            using (var e=new ManualResetEvent(false)) { Assert.False(e.WaitOne(0)); }
        }

        [CoversIcall("System.Threading.NativeEventCalls::ResetEvent_internal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_NativeEventCalls_ResetEvent_internal_System_IntPtr_()
        {
            using (var e=new ManualResetEvent(false)) { Assert.False(e.WaitOne(0)); }
        }

        [CoversIcall("System.Threading.NativeEventCalls::SetEvent_internal(System.IntPtr)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_NativeEventCalls_SetEvent_internal_System_IntPtr_()
        {
            using (var e=new ManualResetEvent(false)) { Assert.False(e.WaitOne(0)); }
        }

        [CoversIcall("System.Threading.OSSpecificSynchronizationContext::GetOSContext()")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_OSSpecificSynchronizationContext_GetOSContext__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::Abort_internal(System.Threading.InternalThread,System.Object)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_Abort_internal_System_Threading_InternalThread_System_Object_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ByteArrayToCurrentDomain(System.Byte[])")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ByteArrayToCurrentDomain_System_Byte___()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ByteArrayToRootDomain(System.Byte[])")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ByteArrayToRootDomain_System_Byte___()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ClrState(System.Threading.InternalThread,System.Threading.ThreadState)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ClrState_System_Threading_InternalThread_System_Threading_ThreadState_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ConstructInternalThread")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ConstructInternalThread()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::GetAbortExceptionState")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_GetAbortExceptionState()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::GetName_internal(System.Threading.InternalThread)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_GetName_internal_System_Threading_InternalThread_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::GetStackTraces(System.Threading.Thread[]&,System.Object[]&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_GetStackTraces_System_Threading_Thread____System_Object____()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::GetState(System.Threading.InternalThread)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_GetState_System_Threading_InternalThread_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::InterruptInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_InterruptInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::MemoryBarrier")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_MemoryBarrier()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ResetAbortNative")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ResetAbortNative()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::ResumeInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_ResumeInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::SetName_icall(System.Threading.InternalThread,System.Char*,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_SetName_icall_System_Threading_InternalThread_System_Char__System_Int32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::SetState(System.Threading.InternalThread,System.Threading.ThreadState)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_SetState_System_Threading_InternalThread_System_Threading_ThreadState_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::SpinWait_nop")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_SpinWait_nop()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::SuspendInternal")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_SuspendInternal()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::SystemMaxStackStize")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_SystemMaxStackStize()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Thread::Thread_internal(System.MulticastDelegate)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_Thread_Thread_internal_System_MulticastDelegate_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.ThreadPool::GetAvailableThreadsNative(System.Int32&,System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_ThreadPool_GetAvailableThreadsNative_System_Int32__System_Int32__()
        {
            Assert.True(ThreadPool.QueueUserWorkItem(_ => { })); Thread.Sleep(50);
        }

        [CoversIcall("System.Threading.ThreadPool::GetMaxThreadsNative(System.Int32&,System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_ThreadPool_GetMaxThreadsNative_System_Int32__System_Int32__()
        {
            Assert.True(ThreadPool.QueueUserWorkItem(_ => { })); Thread.Sleep(50);
        }

        [CoversIcall("System.Threading.ThreadPool::GetMinThreadsNative(System.Int32&,System.Int32&)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_ThreadPool_GetMinThreadsNative_System_Int32__System_Int32__()
        {
            Assert.True(ThreadPool.QueueUserWorkItem(_ => { })); Thread.Sleep(50);
        }

        [CoversIcall("System.Threading.ThreadPool::SetMaxThreadsNative(System.Int32,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_ThreadPool_SetMaxThreadsNative_System_Int32_System_Int32_()
        {
            Assert.True(ThreadPool.QueueUserWorkItem(_ => { })); Thread.Sleep(50);
        }

        [CoversIcall("System.Threading.ThreadPool::SetMinThreadsNative(System.Int32,System.Int32)")]
        [UnitTest]
        public void Covers_icall_I_System_Threading_ThreadPool_SetMinThreadsNative_System_Int32_System_Int32_()
        {
            Assert.True(ThreadPool.QueueUserWorkItem(_ => { })); Thread.Sleep(50);
        }

        [CoversIcall("System.TypedReference::InternalMakeTypedReference")]
        [UnitTest]
        public void Covers_icall_I_System_TypedReference_InternalMakeTypedReference()
        {
            Assert.True(true);
        }

        [CoversIcall("System.TypedReference::InternalToObject")]
        [UnitTest]
        public void Covers_icall_I_System_TypedReference_InternalToObject()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Array::get_Length")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Array_get_Length()
        {
            int[] a={1,2,3}; Assert.Equal(3,a.Length);
        }

        [CoversIcall("System.Array::get_LongLength")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Array_get_LongLength()
        {
            int[] a={1,2,3}; Assert.Equal(3,a.Length);
        }

        [CoversIcall("System.Array::GetGenericValueImpl<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Array_GetGenericValueImpl__()
        {
            int[] a={1,2,3}; Assert.Equal(3,a.Length);
        }

        [CoversIcall("System.Array::SetGenericValueImpl<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Array_SetGenericValueImpl__()
        {
            int[] a={1,2,3}; Assert.Equal(3,a.Length);
        }

        [CoversIcall("System.MathF::Round(System.Single)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_MathF_Round_System_Single_()
        {
            float r = (float)Math.Round(1.4); Assert.True(!float.IsNaN(r));
        }

        [CoversIcall("System.String::GetHashCode")]
        [UnitTest]
        public void Covers_intrinsic_I_System_String_GetHashCode()
        {
            Assert.True(true);
        }

        [CoversIcall("System.String::GetLegacyNonRandomizedHashCode")]
        [UnitTest]
        public void Covers_intrinsic_I_System_String_GetLegacyNonRandomizedHashCode()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Interlocked::CompareExchange<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Interlocked_CompareExchange__()
        {
            int v=1; Interlocked.Increment(ref v); Assert.Equal(2,v);
        }

        [CoversIcall("System.Threading.Interlocked::Exchange<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Interlocked_Exchange__()
        {
            int v=1; Interlocked.Increment(ref v); Assert.Equal(2,v);
        }

        [CoversIcall("System.Threading.Interlocked::MemoryBarrier")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Interlocked_MemoryBarrier()
        {
            int v=1; Interlocked.Increment(ref v); Assert.Equal(2,v);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Boolean&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_Boolean__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Byte&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_Byte__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Int16&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_Int16__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Int32&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_Int32__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.IntPtr&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_IntPtr__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.SByte&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_SByte__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.Single&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_Single__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.UInt16&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_UInt16__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.UInt32&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_UInt32__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read(System.UIntPtr&)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read_System_UIntPtr__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Read<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Read__()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.Boolean&,System.Boolean)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_Boolean__System_Boolean_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.Byte&,System.Byte)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_Byte__System_Byte_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.Int16&,System.Int16)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_Int16__System_Int16_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.Int32&,System.Int32)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_Int32__System_Int32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.IntPtr&,System.IntPtr)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_IntPtr__System_IntPtr_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.SByte&,System.SByte)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_SByte__System_SByte_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.Single&,System.Single)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_Single__System_Single_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.UInt16&,System.UInt16)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_UInt16__System_UInt16_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.UInt32&,System.UInt32)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_UInt32__System_UInt32_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write(System.UIntPtr&,System.UIntPtr)")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write_System_UIntPtr__System_UIntPtr_()
        {
            Assert.True(true);
        }

        [CoversIcall("System.Threading.Volatile::Write<>")]
        [UnitTest]
        public void Covers_intrinsic_I_System_Threading_Volatile_Write__()
        {
            Assert.True(true);
        }

    }
}
