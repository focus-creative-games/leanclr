#include "intrinsic_stubs.h"
#include "system_array.h"
#include "system_object.h"
#include "system_span.h"
#include "intrinsics/system_string.h"
#include "system_threading_interlocked.h"
#include "system_threading_volatile.h"
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
    Append(entries, SystemArray::get_intrinsic_entries());
    Append(entries, SystemObject::get_intrinsic_entries());
    Append(entries, SystemSpan::get_intrinsic_entries());
    Append(entries, SystemString::get_intrinsic_entries());
    Append(entries, SystemThreadingInterlocked::get_intrinsic_entries());
    Append(entries, SystemThreadingVolatile::get_intrinsic_entries());
    Append(entries, SystemNumericsVector::get_intrinsic_entries());
}

void IntrinsicStubs::get_newobj_intrinsic_entries(utils::Vector<vm::NewobjIntrinsicEntry>& entries) noexcept
{
    entries.reserve(200);
    Append(entries, SystemObject::get_newobj_intrinsic_entries());
    // append newobj intrinsic entries from various classes
}
} // namespace intrinsics
} // namespace leanclr
