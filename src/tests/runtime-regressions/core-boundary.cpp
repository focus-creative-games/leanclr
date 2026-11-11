#include "vm/rt_managed_types.h"
#include "vm/reflection.h"
#include "vm/delegate.h"
#include "vm/rt_exception.h"
#include "host/services.h"
#include <type_traits>
#include <cstdlib>

template <typename T, typename = void>
struct IsComplete : std::false_type
{
};

template <typename T>
struct IsComplete<T, decltype(void(sizeof(T)))> : std::true_type
{
};

using namespace leanclr;
static_assert(!IsComplete<vm::RtDelegate>::value && !IsComplete<vm::RtException>::value && !IsComplete<vm::RtReflectionType>::value &&
                  !IsComplete<vm::RtThread>::value && !IsComplete<vm::RtAppDomain>::value,
              "Core headers must keep BCL layouts opaque");
RtResult<vm::RtException*> adapter_result(vm::RtException* object, bool fail);

int main()
{
    vm::RtObject object = {};
    auto* opaque = reinterpret_cast<vm::RtException*>(&object);
    auto ok = adapter_result(opaque, false);
    if (!ok.is_ok() || ok.unwrap() != opaque)
        std::abort();
    auto error = adapter_result(nullptr, true);
    if (!error.is_err() || error.unwrap_err() != RtErr::InvalidCast)
        std::abort();
}
