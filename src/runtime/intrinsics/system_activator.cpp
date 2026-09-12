#include "system_activator.h"

#include <cstring>

#include "vm/class.h"
#include "vm/method.h"
#include "vm/object.h"
#include "vm/runtime.h"
#include "vm/type.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{
RtResult<const metadata::RtTypeSig*> get_create_instance_type_arg(const metadata::RtMethodInfo* method) noexcept
{
    if (method->generic_method != nullptr)
    {
        const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
        if (method_inst != nullptr && method_inst->generic_arg_count == 1)
        {
            RET_OK(method_inst->generic_args[0]);
        }
    }

    if (method->return_type->ele_type != metadata::RtElementType::MVar)
    {
        RET_OK(method->return_type);
    }

    RET_ERR(RtErr::BadImageFormat);
}
} // namespace

RtResultVoid SystemActivator::create_instance(const metadata::RtMethodInfo* method, interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtTypeSig*, type_sig, get_create_instance_type_arg(method));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, is_value_type, vm::Type::is_value_type(type_sig));
    if (is_value_type)
    {
        std::memset(ret, 0, method->ret_stack_object_size * sizeof(interp::RtStackObject));
        RET_VOID_OK();
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj,
                                            LEANCLR_CREATE_INSTANCE_INTERNAL(type_sig, "SystemActivator::create_instance"));
    const metadata::RtMethodInfo* ctor = vm::Method::find_matched_method_in_class_by_name_and_param_count(obj->klass, ".ctor", 0);
    if (ctor != nullptr)
    {
        interp::RtStackObject args[1]{};
        args[0].obj = obj;
        RET_ERR_ON_FAIL(vm::Runtime::invoke_stackobject_arguments_without_run_cctor(ctor, args, nullptr));
    }
    interp::EvalStackOp::set_return(ret, obj);
    RET_VOID_OK();
}

/// @intrinsic: System.Activator::CreateInstance<>()
static RtResultVoid create_instance_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject*, interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemActivator::create_instance(method, ret));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_activator[] = {
    {"System.Activator::CreateInstance<>", nullptr, create_instance_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemActivator::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_activator,
                                           sizeof(s_intrinsic_entries_system_activator) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr
