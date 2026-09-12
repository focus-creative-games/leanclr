#pragma once

namespace leanclr
{

constexpr const char* STR_MODULE = "<Module>";
constexpr const char* STR_SYSTEM = "System";
constexpr const char* STR_VALUETYPE = "ValueType";
constexpr const char* STR_ENUM = "Enum";
constexpr const char* STR_CORLIB_NAME = "mscorlib";
constexpr const char* STR_SYSTEM_PRIVATE_CORELIB_NAME = "System.Private.CoreLib";

constexpr const char* STR_SYSTEM_RUNTIME_INTEROPSERVICES = "System.Runtime.InteropServices";

// Parameter/Attribute names
constexpr const char* STR_INATTRIBUTE = "InAttribute";
constexpr const char* STR_OUTATTRIBUTE = "OutAttribute";
constexpr const char* STR_OPTIONALATTRIBUTE = "OptionalAttribute";

// Method names
constexpr const char* STR_CTOR = ".ctor";
constexpr const char* STR_CCTOR = ".cctor";
constexpr const char* STR_FINALIZE = "Finalize";
constexpr const char* STR_TOSTRING = "ToString";
constexpr const char* STR_EQUALS = "Equals";
constexpr const char* STR_GETHASHCODE = "GetHashCode";

// Delegate method names
constexpr const char* STR_INVOKE = "Invoke";
constexpr const char* STR_BEGININVOKE = "BeginInvoke";
constexpr const char* STR_ENDINVOKE = "EndInvoke";

// Array method names
constexpr const char* STR_ARRAY_GET_NZ = "Get";
constexpr const char* STR_ARRAY_SET_NZ = "Set";
constexpr const char* STR_ARRAY_ADDRESS_NZ = "Address";
} // namespace leanclr
