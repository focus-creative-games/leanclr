#include "internal_call_stubs.h"

#include "mono_runtimeclasshandle.h"
#include "mono_runtimegptrarrayhandle.h"
#include "mono_runtimemarshal.h"
#include "mono_safestringmarshal.h"
#include "system_array.h"
#include "system_object.h"
#include "system_reflection_runtimemethodinfo.h"
#include "system_runtime_compilerservices_runtimehelpers.h"
#include "system_diagnostics_stopwatch.h"
#include "system_runtimetype.h"
#include "system_runtimetypehandle.h"
#include "icalls/system_string.h"
#include "system_globalization_cultureinfo.h"
#include "system_globalization_compareinfo.h"
#include "system_threading_interlocked.h"
#include "system_threading_thread.h"
#include "system_type.h"
#include "system_valuetype.h"
#include "system_environment.h"
#include "system_runtime_interopservices_gchandle.h"
#include "system_runtime_interopservices_marshal.h"
#include "system_runtime_interopservices_runtimeinformation.h"
#include "system_runtime_runtimeimports.h"
#include "system_enum.h"
#include "system_monocustomattrs.h"
#include "system_buffer.h"
#include "system_reflection_fieldinfo.h"
#include "system_reflection_monomethodinfo.h"
#include "system_reflection_runtimeconstructorinfo.h"
#include "system_reflection_runtimefieldinfo.h"
#include "system_reflection_runtimepropertyinfo.h"
#include "system_reflection_assembly.h"
#include "system_reflection_methodbase.h"
#include "system_reflection_runtimeassembly.h"
#include "system_reflection_assemblyname.h"
#include "system_reflection_customattributedata.h"
#include "system_reflection_eventinfo.h"
#include "system_reflection_runtimeeventinfo.h"
#include "system_reflection_runtimeparameterinfo.h"
#include "system_threading_monitor.h"
#include "system_threading_timer.h"
#include "system_threading_nativeeventcalls.h"
#include "system_threading_volatile.h"
#include "system_appdomain.h"
#include "system_delegate.h"
#include "system_argiterator.h"
#include "system_diagnostics_debugger.h"
#include "system_diagnostics_stackframe.h"
#include "system_diagnostics_stacktrace.h"
#include "system_exception.h"
#include "system_reflection_runtimemodule.h"
#include "system_currentsystemtimezone.h"
#include "system_text_encodinghelper.h"
#include "system_security_cryptography_rngcryptoserviceprovider.h"
#include "system_security_securitymanager.h"
#include "system_threading_internalthread.h"
#include "system_typedreference.h"
#include "system_gc.h"
#include "system_datetime.h"
#include "system_math.h"
#include "system_mathf.h"
#include "system_io_path.h"
#include "system_io_monoio.h"
#include "interop.h"

namespace leanclr
{
namespace icalls
{

template <typename T>
static void Append(utils::Vector<T>& entries, const utils::Span<T>& sub_entries) noexcept
{
    entries.push_range(sub_entries.begin(), sub_entries.size());
}

void InternalCallStubs::get_internal_call_entries(utils::Vector<vm::InternalCallEntry>& entries) noexcept
{
    entries.reserve(1000);
    // append all internal call entries here
    Append(entries, MonoRuntimeClassHandle::get_internal_call_entries());
    Append(entries, MonoRuntimeGPtrArrayHandle::get_internal_call_entries());
    Append(entries, MonoRuntimeMarshal::get_internal_call_entries());
    Append(entries, MonoSafeStringMarshal::get_internal_call_entries());
    Append(entries, SystemArray::get_internal_call_entries());
    Append(entries, SystemObject::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeMethodInfo::get_internal_call_entries());
    Append(entries, SystemRuntimeCompilerServicesRuntimeHelpers::get_internal_call_entries());
    Append(entries, SystemDiagnosticsStopwatch::get_internal_call_entries());
    Append(entries, SystemRuntimeType::get_internal_call_entries());
    Append(entries, SystemRuntimeTypeHandle::get_internal_call_entries());
    Append(entries, SystemString::get_internal_call_entries());
    Append(entries, SystemGlobalizationCultureInfo::get_internal_call_entries());
    Append(entries, SystemGlobalizationCompareInfo::get_internal_call_entries());
    Append(entries, SystemEnvironment::get_internal_call_entries());
    Append(entries, SystemThreadingInterlocked::get_internal_call_entries());
    Append(entries, SystemThreadingThread::get_internal_call_entries());
    Append(entries, SystemThreadingInternalThread::get_internal_call_entries());
    Append(entries, SystemType::get_internal_call_entries());
    Append(entries, SystemValueType::get_internal_call_entries());
    Append(entries, SystemTypedReference::get_internal_call_entries());
    Append(entries, SystemRuntimeInteropServicesMarshal::get_internal_call_entries());
    Append(entries, SystemRuntimeInteropServicesGCHandle::get_internal_call_entries());
    Append(entries, SystemRuntimeInteropServicesRuntimeInformation::get_internal_call_entries());
    Append(entries, SystemRuntimeRuntimeImports::get_internal_call_entries());
    Append(entries, Interop::get_internal_call_entries());
    Append(entries, SystemEnum::get_internal_call_entries());
    Append(entries, SystemMonoCustomAttrs::get_internal_call_entries());
    Append(entries, SystemBuffer::get_internal_call_entries());
    Append(entries, SystemReflectionFieldInfo::get_internal_call_entries());
    Append(entries, SystemReflectionMonoMethodInfo::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeConstructorInfo::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeFieldInfo::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimePropertyInfo::get_internal_call_entries());
    Append(entries, SystemReflectionAssembly::get_internal_call_entries());
    Append(entries, SystemReflectionMethodBase::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeAssembly::get_internal_call_entries());
    Append(entries, SystemReflectionAssemblyName::get_internal_call_entries());
    Append(entries, SystemReflectionCustomAttributeData::get_internal_call_entries());
    Append(entries, SystemReflectionEventInfo::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeEventInfo::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeParameterInfo::get_internal_call_entries());
    Append(entries, SystemThreadingMonitor::get_internal_call_entries());
    Append(entries, SystemThreadingTimer::get_internal_call_entries());
    Append(entries, SystemThreadingNativeEventCalls::get_internal_call_entries());
    Append(entries, SystemThreadingVolatile::get_internal_call_entries());
    Append(entries, SystemAppDomain::get_internal_call_entries());
    Append(entries, SystemDelegate::get_internal_call_entries());
    Append(entries, SystemArgIterator::get_internal_call_entries());
    Append(entries, SystemDiagnosticsDebugger::get_internal_call_entries());
    Append(entries, SystemDiagnosticsStackFrame::get_internal_call_entries());
    Append(entries, SystemDiagnosticsStackTrace::get_internal_call_entries());
    Append(entries, SystemException::get_internal_call_entries());
    Append(entries, SystemReflectionRuntimeModule::get_internal_call_entries());
    Append(entries, SystemGC::get_internal_call_entries());
    Append(entries, SystemCurrentSystemTimeZone::get_internal_call_entries());
    Append(entries, SystemDateTime::get_internal_call_entries());
    Append(entries, SystemMath::get_internal_call_entries());
    Append(entries, SystemMathF::get_internal_call_entries());
    Append(entries, SystemIOPath::get_internal_call_entries());
    Append(entries, SystemIOMonoIO::get_internal_call_entries());
    Append(entries, SystemTextEncodingHelper::get_internal_call_entries());
    Append(entries, SystemSecurityCryptographyRNGCryptoServiceProvider::get_internal_call_entries());
    Append(entries, SystemSecuritySecurityManager::get_internal_call_entries());
}

void InternalCallStubs::get_newobj_internal_call_entries(utils::Vector<vm::NewobjInternalCallEntry>& entries) noexcept
{
    entries.reserve(200);
    // append all newobj internal call entries here
    Append(entries, SystemString::get_newobj_internal_call_entries());
}

} // namespace icalls
} // namespace leanclr
