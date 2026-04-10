#pragma once

namespace leanclr
{
namespace core
{
enum class RtErr : uint8_t
{
    None = 0,
    ManagedException,
    ExceedMaxImageCount,
    NotImplemented,
    StackOverflow,
    InvalidCast,
    NullReference,
    ArrayTypeMismatch,
    IndexOutOfRange,
    OutOfMemory,
    Arithmetic,
    FieldAccess,
    MethodAccess,
    ExecutionEngine,
    ArgumentNull,
    DivideByZero,
    Overflow,
    TypeLoad,
    MissingField,
    MissingMethod,
    MissingMember,
    BadImageFormat,
    EntryPointNotFound,
    NotSupported,
    ArgumentOutOfRange,
    Argument,
    FileNotFound,
    InvalidOperation,
    ModuleAlreadyLoaded,
};
} // namespace core
} // namespace leanclr
