#include "system_signature.h"

#include "utils/binary_reader.h"

namespace leanclr
{
namespace icalls
{
namespace
{
constexpr uint8_t CALLCONV_FIELD = 0x06;
constexpr uint8_t CALLCONV_GENERIC = 0x10;
constexpr uint8_t CALLCONV_MASK = 0x0F;

RtResultVoid skip_type_signature(utils::BinaryReader& reader) noexcept;

RtResultVoid read_compressed_u32(utils::BinaryReader& reader, uint32_t& value) noexcept
{
    if (!reader.try_read_compressed_uint32(value))
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    RET_VOID_OK();
}

RtResultVoid read_compressed_i32(utils::BinaryReader& reader, int32_t& value) noexcept
{
    if (!reader.try_read_compressed_int32(value))
    {
        RET_ERR(RtErr::BadImageFormat);
    }
    RET_VOID_OK();
}

RtResultVoid skip_method_signature(utils::BinaryReader& reader) noexcept
{
    uint8_t call_conv = 0;
    if (!reader.try_read_byte(call_conv))
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    if ((call_conv & CALLCONV_GENERIC) != 0)
    {
        uint32_t generic_param_count = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, generic_param_count));
    }

    uint32_t parameter_count = 0;
    RET_ERR_ON_FAIL(read_compressed_u32(reader, parameter_count));
    RET_ERR_ON_FAIL(skip_type_signature(reader));
    for (uint32_t i = 0; i < parameter_count; ++i)
    {
        RET_ERR_ON_FAIL(skip_type_signature(reader));
    }

    RET_VOID_OK();
}

RtResultVoid skip_custom_modifiers(utils::BinaryReader& reader, uint8_t& element_type) noexcept
{
    while (element_type == 0x1F || element_type == 0x20)
    {
        uint32_t coded_type_token = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, coded_type_token));
        if (!reader.try_read_byte(element_type))
        {
            RET_ERR(RtErr::BadImageFormat);
        }
    }

    RET_VOID_OK();
}

RtResultVoid skip_type_signature(utils::BinaryReader& reader) noexcept
{
    uint8_t element_type = 0;
    if (!reader.try_read_byte(element_type))
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    RET_ERR_ON_FAIL(skip_custom_modifiers(reader, element_type));

    if (element_type == 0x41 || element_type == 0x45)
    {
        return skip_type_signature(reader);
    }

    switch (element_type)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x16:
    case 0x18:
    case 0x19:
    case 0x1C:
        RET_VOID_OK();

    case 0x0F:
    case 0x10:
    case 0x1D:
        return skip_type_signature(reader);

    case 0x11:
    case 0x12:
    case 0x13:
    case 0x1E:
    {
        uint32_t token_or_index = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, token_or_index));
        RET_VOID_OK();
    }

    case 0x14:
    {
        RET_ERR_ON_FAIL(skip_type_signature(reader));
        uint32_t rank = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, rank));
        uint32_t size_count = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, size_count));
        for (uint32_t i = 0; i < size_count; ++i)
        {
            uint32_t size = 0;
            RET_ERR_ON_FAIL(read_compressed_u32(reader, size));
        }
        uint32_t lower_bound_count = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, lower_bound_count));
        for (uint32_t i = 0; i < lower_bound_count; ++i)
        {
            int32_t lower_bound = 0;
            RET_ERR_ON_FAIL(read_compressed_i32(reader, lower_bound));
        }
        RET_VOID_OK();
    }

    case 0x15:
    {
        uint8_t class_or_value_type = 0;
        if (!reader.try_read_byte(class_or_value_type))
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        if (class_or_value_type != 0x11 && class_or_value_type != 0x12)
        {
            RET_ERR(RtErr::BadImageFormat);
        }
        uint32_t type_def_or_ref = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, type_def_or_ref));
        uint32_t arg_count = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, arg_count));
        for (uint32_t i = 0; i < arg_count; ++i)
        {
            RET_ERR_ON_FAIL(skip_type_signature(reader));
        }
        RET_VOID_OK();
    }

    case 0x1B:
        return skip_method_signature(reader);

    default:
        RET_ERR(RtErr::BadImageFormat);
    }
}

} // namespace

RtResult<int32_t> SystemSignature::get_parameter_offset_internal(void* sig, int32_t csig, int32_t parameter_index) noexcept
{
    if (sig == nullptr || csig <= 0 || parameter_index < 0)
    {
        RET_ERR(RtErr::Argument);
    }

    utils::BinaryReader reader(sig, static_cast<size_t>(csig));
    uint8_t call_conv = 0;
    if (!reader.try_read_byte(call_conv))
    {
        RET_ERR(RtErr::BadImageFormat);
    }

    if ((call_conv & CALLCONV_MASK) == CALLCONV_FIELD)
    {
        if (parameter_index != 0)
        {
            RET_ERR(RtErr::ArgumentOutOfRange);
        }
        RET_OK(static_cast<int32_t>(reader.get_position()));
    }

    if ((call_conv & CALLCONV_GENERIC) != 0)
    {
        uint32_t generic_param_count = 0;
        RET_ERR_ON_FAIL(read_compressed_u32(reader, generic_param_count));
    }

    uint32_t parameter_count = 0;
    RET_ERR_ON_FAIL(read_compressed_u32(reader, parameter_count));
    if (parameter_index > static_cast<int32_t>(parameter_count))
    {
        RET_ERR(RtErr::ArgumentOutOfRange);
    }

    for (int32_t i = 0; i < parameter_index; ++i)
    {
        RET_ERR_ON_FAIL(skip_type_signature(reader));
    }

    RET_OK(static_cast<int32_t>(reader.get_position()));
}

static RtResultVoid get_parameter_offset_internal_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*,
                                                          const interp::RtStackObject* params, interp::RtStackObject* ret) noexcept
{
    auto sig = EvalStackOp::get_param<void*>(params, 0);
    int32_t csig = EvalStackOp::get_param<int32_t>(params, 1);
    int32_t parameter_index = EvalStackOp::get_param<int32_t>(params, 2);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(int32_t, offset,
                                            SystemSignature::get_parameter_offset_internal(sig, csig, parameter_index));
    EvalStackOp::set_return(ret, offset);
    RET_VOID_OK();
}

utils::Span<vm::InternalCallEntry> SystemSignature::get_internal_call_entries() noexcept
{
    static vm::InternalCallEntry s_entries[] = {
        {"System.Signature::GetParameterOffsetInternal(System.Void*,System.Int32,System.Int32)",
         (vm::InternalCallFunction)&SystemSignature::get_parameter_offset_internal, get_parameter_offset_internal_invoker},
        {"System.Signature::GetParameterOffsetInternal",
         (vm::InternalCallFunction)&SystemSignature::get_parameter_offset_internal, get_parameter_offset_internal_invoker},
    };
    return utils::Span<vm::InternalCallEntry>(s_entries, sizeof(s_entries) / sizeof(s_entries[0]));
}

} // namespace icalls
} // namespace leanclr
