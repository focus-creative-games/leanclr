#include "system_runtime_compilerservices_unsafe.h"

#include <cstring>
#include <limits>

#include "interp/eval_stack_op.h"
#include "interp/interp_defs.h"

namespace leanclr
{
namespace intrinsics
{
namespace
{

RtResult<interp::ReduceTypeAndSize> get_first_generic_arg_type_and_size(const metadata::RtMethodInfo* method) noexcept
{
    if (method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (method_inst->generic_arg_count < 1)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size,
                                            interp::InterpDefs::get_reduce_type_and_size_by_typesig(method_inst->generic_args[0]));
    RET_OK(type_and_size);
}

RtResult<size_t> get_first_generic_arg_size(const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size, get_first_generic_arg_type_and_size(method));
    RET_OK(type_and_size.byte_size);
}

intptr_t get_integer_offset_param(const metadata::RtMethodInfo* method, const interp::RtStackObject* params, size_t index) noexcept
{
    const metadata::RtTypeSig* param_type = method->parameters[index];
    switch (param_type->ele_type)
    {
    case metadata::RtElementType::I4:
        return static_cast<intptr_t>(interp::EvalStackOp::get_param<int32_t>(params, index));
    case metadata::RtElementType::U4:
        return static_cast<intptr_t>(interp::EvalStackOp::get_param<uint32_t>(params, index));
    case metadata::RtElementType::U:
        return static_cast<intptr_t>(interp::EvalStackOp::get_param<uintptr_t>(params, index));
    case metadata::RtElementType::I:
    default:
        return static_cast<intptr_t>(interp::EvalStackOp::get_param<intptr_t>(params, index));
    }
}

void* add_bytes_to_pointer(void* source, intptr_t byte_offset) noexcept
{
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(source) + static_cast<uintptr_t>(byte_offset));
}

} // namespace

RtResult<void*> SystemRuntimeCompilerServicesUnsafe::as_pointer(void* location) noexcept
{
    RET_OK(location);
}

RtResult<void*> SystemRuntimeCompilerServicesUnsafe::as(void* source) noexcept
{
    RET_OK(source);
}

RtResult<intptr_t> SystemRuntimeCompilerServicesUnsafe::byte_offset(void* origin, void* target) noexcept
{
    const uintptr_t origin_addr = reinterpret_cast<uintptr_t>(origin);
    const uintptr_t target_addr = reinterpret_cast<uintptr_t>(target);
    RET_OK(static_cast<intptr_t>(target_addr - origin_addr));
}

RtResult<bool> SystemRuntimeCompilerServicesUnsafe::are_same(void* left, void* right) noexcept
{
    RET_OK(left == right);
}

RtResult<bool> SystemRuntimeCompilerServicesUnsafe::is_address_less_than(void* left, void* right) noexcept
{
    RET_OK(reinterpret_cast<uintptr_t>(left) < reinterpret_cast<uintptr_t>(right));
}

RtResult<bool> SystemRuntimeCompilerServicesUnsafe::is_address_greater_than(void* left, void* right) noexcept
{
    RET_OK(reinterpret_cast<uintptr_t>(left) > reinterpret_cast<uintptr_t>(right));
}

RtResult<int32_t> SystemRuntimeCompilerServicesUnsafe::size_of(const metadata::RtMethodInfo* method) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, value_size, get_first_generic_arg_size(method));
    if (value_size > static_cast<size_t>(std::numeric_limits<int32_t>::max()))
    {
        RET_ERR(RtErr::Overflow);
    }
    RET_OK(static_cast<int32_t>(value_size));
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::copy_block(const interp::RtStackObject* params) noexcept
{
    void* destination = interp::EvalStackOp::get_param<void*>(params, 0);
    void* source = interp::EvalStackOp::get_param<void*>(params, 1);
    uint32_t byte_count = interp::EvalStackOp::get_param<uint32_t>(params, 2);
    std::memmove(destination, source, byte_count);
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::read_unaligned(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                 interp::RtStackObject* ret) noexcept
{
    void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, type_and_size, get_first_generic_arg_type_and_size(method));

    switch (type_and_size.reduce_type)
    {
    case metadata::RtArgOrLocOrFieldReduceType::I1:
    {
        int8_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::U1:
    {
        uint8_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::I2:
    {
        int16_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::U2:
    {
        uint16_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, static_cast<int32_t>(value));
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::I4:
    {
        int32_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::I8:
    {
        int64_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::I:
    {
        intptr_t value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::R4:
    {
        float value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::R8:
    {
        double value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::Ref:
    {
        void* value;
        std::memcpy(&value, source, sizeof(value));
        interp::EvalStackOp::set_return(ret, value);
        break;
    }
    case metadata::RtArgOrLocOrFieldReduceType::Other:
    {
        std::memcpy(ret, source, type_and_size.byte_size);
        break;
    }
    default:
        RET_ERR(RtErr::ExecutionEngine);
    }

    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::write_unaligned(const metadata::RtMethodInfo* method, const interp::RtStackObject* params) noexcept
{
    void* destination = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, value_size, get_first_generic_arg_size(method));
    std::memcpy(destination, params + 1, value_size);
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::add(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                      interp::RtStackObject* ret) noexcept
{
    void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    intptr_t element_offset = get_integer_offset_param(method, params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(size_t, element_size, get_first_generic_arg_size(method));
    interp::EvalStackOp::set_return(ret, add_bytes_to_pointer(source, element_offset * static_cast<intptr_t>(element_size)));
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::add_byte_offset(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                  interp::RtStackObject* ret) noexcept
{
    void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    intptr_t byte_offset = get_integer_offset_param(method, params, 1);
    interp::EvalStackOp::set_return(ret, add_bytes_to_pointer(source, byte_offset));
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::subtract_byte_offset(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                       interp::RtStackObject* ret) noexcept
{
    void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    intptr_t byte_offset = get_integer_offset_param(method, params, 1);
    interp::EvalStackOp::set_return(ret, add_bytes_to_pointer(source, -byte_offset));
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::skip_init(const interp::RtStackObject*) noexcept
{
    // CoreCLR treats SkipInit<T>(out T) as a JIT hint. LeanCLR locals are already initialized by the interpreter.
    RET_VOID_OK();
}

RtResultVoid SystemRuntimeCompilerServicesUnsafe::bit_cast(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                           interp::RtStackObject* ret) noexcept
{
    if (method == nullptr || method->generic_method == nullptr || method->generic_method->generic_context.method_inst == nullptr)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    const metadata::RtGenericInst* method_inst = method->generic_method->generic_context.method_inst;
    if (method_inst->generic_arg_count != 2)
    {
        RET_ERR(RtErr::ExecutionEngine);
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, source_type_and_size,
                                            interp::InterpDefs::get_reduce_type_and_size_by_typesig(method_inst->generic_args[0]));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, target_type_and_size,
                                            interp::InterpDefs::get_reduce_type_and_size_by_typesig(method_inst->generic_args[1]));

    if (source_type_and_size.byte_size != target_type_and_size.byte_size)
    {
        RET_ERR(RtErr::NotSupported);
    }

    std::memcpy(ret, params, source_type_and_size.byte_size);
    RET_VOID_OK();
}

static RtResultVoid as_pointer_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject* ret) noexcept
{
    void* location = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, result, SystemRuntimeCompilerServicesUnsafe::as_pointer(location));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid as_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                               interp::RtStackObject* ret) noexcept
{
    void* source = interp::EvalStackOp::get_param<void*>(params, 0);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(void*, result, SystemRuntimeCompilerServicesUnsafe::as(source));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid bit_cast_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::bit_cast(method, params, ret));
    RET_VOID_OK();
}

static RtResultVoid byte_offset_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                        interp::RtStackObject* ret) noexcept
{
    void* origin = interp::EvalStackOp::get_param<void*>(params, 0);
    void* target = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(intptr_t, result, SystemRuntimeCompilerServicesUnsafe::byte_offset(origin, target));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid are_same_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                     interp::RtStackObject* ret) noexcept
{
    void* left = interp::EvalStackOp::get_param<void*>(params, 0);
    void* right = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeCompilerServicesUnsafe::are_same(left, right));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static RtResultVoid is_address_less_than_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                 interp::RtStackObject* ret) noexcept
{
    void* left = interp::EvalStackOp::get_param<void*>(params, 0);
    void* right = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeCompilerServicesUnsafe::is_address_less_than(left, right));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static RtResultVoid is_address_greater_than_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                                    interp::RtStackObject* ret) noexcept
{
    void* left = interp::EvalStackOp::get_param<void*>(params, 0);
    void* right = interp::EvalStackOp::get_param<void*>(params, 1);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(bool, result, SystemRuntimeCompilerServicesUnsafe::is_address_greater_than(left, right));
    interp::EvalStackOp::set_return(ret, static_cast<int32_t>(result));
    RET_VOID_OK();
}

static RtResultVoid size_of_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method, const interp::RtStackObject*,
                                    interp::RtStackObject* ret) noexcept
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, result, SystemRuntimeCompilerServicesUnsafe::size_of(method));
    interp::EvalStackOp::set_return(ret, result);
    RET_VOID_OK();
}

static RtResultVoid copy_block_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                       interp::RtStackObject*) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::copy_block(params));
    RET_VOID_OK();
}

static RtResultVoid read_unaligned_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                           const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::read_unaligned(method, params, ret));
    RET_VOID_OK();
}

static RtResultVoid write_unaligned_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                            const interp::RtStackObject* params, interp::RtStackObject*) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::write_unaligned(method, params));
    RET_VOID_OK();
}

static RtResultVoid add_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::add(method, params, ret));
    RET_VOID_OK();
}

static RtResultVoid add_byte_offset_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                           interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::add_byte_offset(method, params, ret));
    RET_VOID_OK();
}

static RtResultVoid subtract_byte_offset_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo* method,
                                                const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::subtract_byte_offset(method, params, ret));
    RET_VOID_OK();
}

static RtResultVoid skip_init_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params,
                                      interp::RtStackObject*) noexcept
{
    RET_ERR_ON_FAIL(SystemRuntimeCompilerServicesUnsafe::skip_init(params));
    RET_VOID_OK();
}

static vm::IntrinsicEntry s_intrinsic_entries_system_runtime_compilerservices_unsafe[] = {
    {"System.Runtime.CompilerServices.Unsafe::AsPointer<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::as_pointer, as_pointer_invoker},
    {"System.Runtime.CompilerServices.Unsafe::AsRef<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::as_pointer, as_pointer_invoker},
    {"System.Runtime.CompilerServices.Unsafe::As<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::as, as_invoker},
    {"System.Runtime.CompilerServices.Unsafe::As<,>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::as, as_invoker},
    {"System.Runtime.CompilerServices.Unsafe::ByteOffset<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::byte_offset, byte_offset_invoker},
    {"System.Runtime.CompilerServices.Unsafe::AreSame<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::are_same, are_same_invoker},
    {"System.Runtime.CompilerServices.Unsafe::IsAddressLessThan<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::is_address_less_than,
     is_address_less_than_invoker},
    {"System.Runtime.CompilerServices.Unsafe::IsAddressGreaterThan<>",
     (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::is_address_greater_than, is_address_greater_than_invoker},
    {"System.Runtime.CompilerServices.Unsafe::SizeOf<>", (vm::IntrinsicFunction)&SystemRuntimeCompilerServicesUnsafe::size_of, size_of_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlock(System.Void*,System.Void*,System.UInt32)", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlock(System.Byte&,System.Byte&,System.UInt32)", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlock", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlockUnaligned(System.Void*,System.Void*,System.UInt32)", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlockUnaligned(System.Byte&,System.Byte&,System.UInt32)", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::CopyBlockUnaligned", nullptr, copy_block_invoker},
    {"System.Runtime.CompilerServices.Unsafe::ReadUnaligned<>(System.Void*)", nullptr, read_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::ReadUnaligned<>(System.Byte&)", nullptr, read_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::ReadUnaligned<>", nullptr, read_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::WriteUnaligned<>(System.Void*,T)", nullptr, write_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::WriteUnaligned<>(System.Byte&,T)", nullptr, write_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::WriteUnaligned<>", nullptr, write_unaligned_invoker},
    {"System.Runtime.CompilerServices.Unsafe::Add<>", nullptr, add_invoker},
    {"System.Runtime.CompilerServices.Unsafe::AddByteOffset<>", nullptr, add_byte_offset_invoker},
    {"System.Runtime.CompilerServices.Unsafe::SubtractByteOffset<>", nullptr, subtract_byte_offset_invoker},
    {"System.Runtime.CompilerServices.Unsafe::SkipInit<>", nullptr, skip_init_invoker},
    {"System.Runtime.CompilerServices.Unsafe::SkipInit", nullptr, skip_init_invoker},
    {"System.Runtime.CompilerServices.Unsafe::BitCast<,>", nullptr, bit_cast_invoker},
};

utils::Span<vm::IntrinsicEntry> SystemRuntimeCompilerServicesUnsafe::get_intrinsic_entries() noexcept
{
    return utils::Span<vm::IntrinsicEntry>(s_intrinsic_entries_system_runtime_compilerservices_unsafe,
                                           sizeof(s_intrinsic_entries_system_runtime_compilerservices_unsafe) / sizeof(vm::IntrinsicEntry));
}

} // namespace intrinsics
} // namespace leanclr
