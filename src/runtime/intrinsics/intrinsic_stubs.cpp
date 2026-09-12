#include "intrinsic_stubs.h"
#include "system_activator.h"
#include "system_array.h"
#include "system_buffer.h"
#include "system_datetime.h"
#include "system_object.h"
#include "system_reflection_customattribute.h"
#include "system_reflection_rtfieldinfo.h"
#include "system_span.h"
#include "system_sr.h"
#include "system_runtime_compilerservices_unsafe.h"
#include "system_runtime_compilerservices_runtimehelpers.h"
#include "system_runtime_compilerservices_casthelpers.h"
#include "system_runtime_compilerservices_yieldawaiter.h"
#include "system_runtime_interopservices_marshal.h"
#include "system_runtime_interopservices_memorymarshal.h"
#include "system_runtimetype.h"
#include "system_runtimetypehandle.h"
#include "system_type.h"
#include "system_valuetype.h"
#include "intrinsics/system_string.h"
#include "system_mathf.h"
#include "system_threading_interlocked.h"
#include "system_threading_volatile.h"
#include "system_threading_thread_intrinsics.h"
#include "system_numerics_vector.h"

namespace leanclr
{
namespace intrinsics
{
template <typename T>
static void Append(utils::Vector<T>& entries, const utils::Span<T>& sub_entries) noexcept
{
    entries.push_range(sub_entries.begin(), sub_entries.size());
}

void IntrinsicStubs::get_intrinsic_entries(utils::Vector<vm::IntrinsicEntry>& entries) noexcept
{
    entries.reserve(1000);
    // append intrinsic entries from various classes
    Append(entries, SystemActivator::get_intrinsic_entries());
    Append(entries, SystemArray::get_intrinsic_entries());
    Append(entries, SystemBuffer::get_intrinsic_entries());
    Append(entries, SystemDateTime::get_intrinsic_entries());
    Append(entries, SystemObject::get_intrinsic_entries());
    Append(entries, SystemReflectionCustomAttribute::get_intrinsic_entries());
    Append(entries, SystemReflectionRtFieldInfo::get_intrinsic_entries());
    Append(entries, SystemSpan::get_intrinsic_entries());
    Append(entries, SystemSR::get_intrinsic_entries());
    Append(entries, SystemRuntimeCompilerServicesUnsafe::get_intrinsic_entries());
    Append(entries, SystemRuntimeCompilerServicesRuntimeHelpers::get_intrinsic_entries());
    Append(entries, SystemRuntimeCompilerServicesCastHelpers::get_intrinsic_entries());
    Append(entries, SystemRuntimeCompilerServicesYieldAwaiter::get_intrinsic_entries());
    Append(entries, SystemRuntimeInteropServicesMarshal::get_intrinsic_entries());
    Append(entries, SystemRuntimeInteropServicesMemoryMarshal::get_intrinsic_entries());
    Append(entries, SystemRuntimeType::get_intrinsic_entries());
    Append(entries, SystemRuntimeTypeHandle::get_intrinsic_entries());
    Append(entries, SystemType::get_intrinsic_entries());
    Append(entries, SystemValueType::get_intrinsic_entries());
    Append(entries, SystemString::get_intrinsic_entries());
    Append(entries, SystemMathF::get_intrinsic_entries());
    Append(entries, SystemThreadingInterlocked::get_intrinsic_entries());
    Append(entries, SystemThreadingVolatile::get_intrinsic_entries());
    Append(entries, SystemThreadingThreadIntrinsics::get_intrinsic_entries());
    Append(entries, SystemNumericsVector::get_intrinsic_entries());
}

void IntrinsicStubs::get_newobj_intrinsic_entries(utils::Vector<vm::NewobjIntrinsicEntry>& entries) noexcept
{
    entries.reserve(200);
    Append(entries, SystemObject::get_newobj_intrinsic_entries());
    Append(entries, SystemSpan::get_newobj_intrinsic_entries());
    // append newobj intrinsic entries from various classes
}
} // namespace intrinsics
} // namespace leanclr
