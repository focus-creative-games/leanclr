#include "liveness.h"
#include "vm/class.h"

namespace leanclr
{
namespace il2cpp
{

struct LivenessState
{
    metadata::RtClass* filter;
    uint32_t max_object_count;
    il2cpp_register_object_callback callback;
    void* userdata;
    il2cpp_liveness_reallocate_callback reallocate;

    LivenessState(metadata::RtClass* filter, uint32_t max_object_count, il2cpp_register_object_callback callback, void* userdata,
                  il2cpp_liveness_reallocate_callback reallocate)
        : filter(filter), max_object_count(max_object_count), callback(callback), userdata(userdata), reallocate(reallocate)
    {
    }

    void finalize()
    {

    }
};

void* Liveness::allocate_struct(metadata::RtClass* filter, uint32_t max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                il2cpp_liveness_reallocate_callback reallocate)
{
    if (filter)
    {
        auto ret = vm::Class::initialize_super_types(filter);
        if (ret.is_err())
        {
            return nullptr;
        }
    }
    return new LivenessState(filter, max_object_count, callback, userdata, reallocate);
}

void Liveness::free_struct(void* state)
{
    delete reinterpret_cast<LivenessState*>(state);
}

void Liveness::calculation_from_root(Il2CppObject* root, void* state)
{
}

void Liveness::calculation_from_statics(void* state)
{
}

void Liveness::finalize(void* state)
{
    reinterpret_cast<LivenessState*>(state)->finalize();
}
} // namespace il2cpp
} // namespace leanclr