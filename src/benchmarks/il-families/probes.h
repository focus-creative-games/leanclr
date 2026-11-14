#pragma once

// Diagnostic specializations for the measured kernels, not production implementations.
#include "bcl/mono/managed_layouts.h"
#include "codegen/leanclr_common.h"
#include "vm/array_class.h"

leanclr::RtResult<uint32_t> leanclr_generated_direct_call_bridge_97da025c8ea411d126916fab24ea5c5a(leanclr::vm::RtObject*, uint32_t,
                                                                                                  const leanclr::metadata::RtMethodInfo*, bool) noexcept;

inline leanclr::RtResult<uint32_t> probe_delegate(leanclr::vm::RtObject* obj, uint32_t arg, const leanclr::metadata::RtMethodInfo* invoke,
                                                  bool callvir) noexcept
{
    using namespace leanclr;
    if (obj)
    {
        auto* del = reinterpret_cast<vm::RtMulticastDelegate*>(obj);
        const auto* method = del->dele.method;
        if (!del->deles && !del->dele.method_is_virtual && method && vm::Method::is_static(method) && method->parameter_count == 1 &&
            method->invoker_type == metadata::RtInvokerType::Aot && !method->generic_method && method->aot_return_protocol == 0 &&
            !vm::Class::is_cctor_not_finished(method->parent))
        {
            return reinterpret_cast<RtResult<uint32_t> (*)(uint32_t) noexcept>(method->method_ptr)(arg);
        }
    }
    return leanclr_generated_direct_call_bridge_97da025c8ea411d126916fab24ea5c5a(obj, arg, invoke, callvir);
}

inline leanclr::vm::RtObject* probe_cast(leanclr::vm::RtObject* obj, const leanclr::metadata::RtClass* target) noexcept
{
    using namespace leanclr;
    if (!obj)
        return nullptr;
    if (target->by_val->ele_type == metadata::RtElementType::Class && !vm::Class::is_interface(target))
    {
        if (vm::Class::is_sealed(target))
            return obj->klass == target ? obj : nullptr;
        if (vm::Class::has_initialized_part(obj->klass, metadata::RtClassInitPart::SuperTypes))
            return vm::Class::has_class_parent_fast(obj->klass, target) ? obj : nullptr;
    }
    return codegen::is_inst(obj, target);
}

inline leanclr::RtResult<leanclr::vm::RtArray*> probe_array(const leanclr::metadata::RtClass* element, int32_t length, const char*) noexcept
{
    using namespace leanclr;
    // One measured allocation site, one element type and constant length.
    static metadata::RtClass* cached_class = nullptr;
    static const metadata::RtClass* cached_element = nullptr;
    static size_t cached_size = 0;
    if (length != 16 || (cached_element && cached_element != element))
        return vm::Array::__new_szarray_from_ele_klass(element, length);
    if (!cached_class)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::ArrayClass::get_szarray_class_from_element_class(element));
        RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::ArrayLayout, layout, vm::Array::get_array_layout(klass, length));
        cached_element = element;
        cached_size = layout.allocation_size;
        cached_class = klass;
    }
#if PROBE_ARRAY_LEVEL == 1
    return vm::Array::__new_szarray_from_array_klass(cached_class, length);
#else
    auto* arr = reinterpret_cast<vm::RtArray*>(gc::GarbageCollector::allocate_array(cached_class, cached_size));
    if (!arr)
        RET_ERR(RtErr::OutOfMemory);
    arr->length = length;
    RET_OK(arr);
#endif
}
