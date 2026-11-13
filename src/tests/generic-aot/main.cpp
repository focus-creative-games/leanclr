#include "host/services.h"
#include "alloc/general_allocation.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/generic_method.h"
#include "vm/generic_class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include "vm/object.h"
#include "vm/rt_string.h"
#include "vm/rt_exception.h"
#include "codegen/aot_result.h"
#include "../aot-returns/test_assert.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

using namespace leanclr;
extern metadata::RtAotModulesData g_aot_modules_data;
static std::string dlls;
static bool injected_error;

static RtResult<vm::RtObject*> legacy_reference(vm::RtObject*) noexcept
{
    if (injected_error)
        return RtErr::OutOfMemory;
    return static_cast<vm::RtObject*>(nullptr);
}

static codegen::AotPointerResult<vm::RtObject*> scalar_reference(vm::RtObject*) noexcept
{
    if (injected_error)
        return RtErr::OutOfMemory;
    return static_cast<vm::RtObject*>(nullptr);
}

static RtResultVoid legacy_void(vm::RtObject*) noexcept
{
    if (injected_error)
        return RtErr::OutOfMemory;
    return core::Unit{};
}

static codegen::AotVoidResult scalar_void(vm::RtObject*) noexcept
{
    if (injected_error)
        return RtErr::OutOfMemory;
    return core::Unit{};
}

static void check(bool ok, const char* message)
{
    if (!ok)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool load(const char* name, const char* ext, host::FileData& data)
{
    std::ifstream file(dlls + "/" + name + "." + ext, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    const auto length = file.tellg();
    if (length < 0)
        return false;
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
    if (!bytes)
        return false;
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes), length))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(length), false};
    return true;
}

static const metadata::RtMethodInfo* inflate(metadata::RtClass* klass, const char* name, const metadata::RtTypeSig* arg)
{
    auto* def = vm::Method::find_matched_method_in_class_by_name(klass, name);
    check(def != nullptr, name);
    auto inst = metadata::MetadataCache::get_pooled_generic_inst(&arg, 1);
    check(inst.is_ok(), "pool generic arguments");
    auto method = vm::GenericMethod::get_method(def, nullptr, inst.unwrap());
    check(method.is_ok(), "inflate method");
    return method.unwrap();
}

static int32_t invoke_int(const metadata::RtMethodInfo* method, const void** args)
{
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(result.is_ok() && result.unwrap(), "invoke closed generic method");
    return *reinterpret_cast<const int32_t*>(result.unwrap() + 1);
}

template <typename T>
static RtResult<T> legacy_integer(vm::RtObject*, T value) noexcept
{
    if (injected_error)
        return RtErr::OutOfMemory;
    return value;
}

template <typename T>
static codegen::AotInt32Result<T> packed_integer(vm::RtObject* object, T value) noexcept
{
    return legacy_integer(object, value);
}

static RtResult<int32_t> legacy_generic_integer(vm::RtObject*, const metadata::RtMethodInfo* context) noexcept
{
    check(context && context->generic_method && context->generic_method->generic_context.class_inst, "generic mixed ABI context missing");
    if (injected_error)
        return RtErr::OutOfMemory;
    return INT32_MIN;
}

static codegen::AotInt32Result<int32_t> packed_generic_integer(vm::RtObject* object, const metadata::RtMethodInfo* context) noexcept
{
    return legacy_generic_integer(object, context);
}

static void integer_bridge_checks(metadata::RtClass* fixture)
{
    for (const char* name : {"CodeReader", "IntegerReader", "UnsignedReader"})
    {
        auto nested = vm::Class::find_nested_class_by_name(fixture, name, false);
        check(nested.is_ok() && nested.unwrap() && vm::Class::initialize_all(nested.unwrap()).is_ok(), "integer reader type");
        auto object = LEANCLR_NEWOBJ(nested.unwrap(), gc::GcAllocSite::make_internal(__FILE__, __LINE__, "mixed integer ABI"));
        check(object.is_ok(), "integer reader allocation");
        auto* target = const_cast<metadata::RtMethodInfo*>(vm::Method::find_matched_method_in_class_by_name(nested.unwrap(), "Read"));
        const auto original = *target;
        const bool enumeration = std::strcmp(name, "CodeReader") == 0;
        const bool unsigned_value = std::strcmp(name, "UnsignedReader") == 0;
        const auto* bridge = vm::Method::find_matched_method_in_class_by_name(fixture, enumeration      ? "EnumBridge"
                                                                                       : unsigned_value ? "UnsignedBridge"
                                                                                                        : "IntegerBridge");
        for (uint8_t flags = 0; flags < 4; ++flags)
        {
            target->aot_return_protocol = flags;
            const bool packed = !enumeration && (flags & 2);
            target->method_ptr = target->virtual_method_ptr = unsigned_value
                                                                  ? (packed ? reinterpret_cast<metadata::RtManagedMethodPointer>(packed_integer<uint32_t>)
                                                                            : reinterpret_cast<metadata::RtManagedMethodPointer>(legacy_integer<uint32_t>))
                                                                  : (packed ? reinterpret_cast<metadata::RtManagedMethodPointer>(packed_integer<int32_t>)
                                                                            : reinterpret_cast<metadata::RtManagedMethodPointer>(legacy_integer<int32_t>));
            for (uint32_t value : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu})
            {
                const void* args[] = {object.unwrap(), &value};
                injected_error = false;
                auto success = vm::Runtime::invoke_with_run_cctor(bridge, nullptr, args);
                check(success.is_ok() && success.unwrap() && *reinterpret_cast<const uint32_t*>(success.unwrap() + 1) == value,
                      "mixed integer/enum value lost");
                injected_error = true;
                auto failure = vm::Runtime::invoke_with_run_cctor(bridge, nullptr, args);
                check(failure.is_err() && failure.unwrap_err() == RtErr::ManagedException && vm::Exception::get_current_exception() &&
                          reinterpret_cast<vm::RtObject*>(vm::Exception::get_current_exception())->klass ==
                              vm::Class::get_corlib_types().cls_out_of_memory_exception,
                      "mixed integer OOM propagation");
            }
        }
        *target = original;
    }
    auto generic_def = vm::Class::find_nested_class_by_name(fixture, "GenericReader`1", false);
    check(generic_def.is_ok() && generic_def.unwrap(), "generic mixed ABI reader");
    const auto* argument = vm::Class::get_corlib_types().cls_int32->by_val;
    auto inst = metadata::MetadataCache::get_pooled_generic_inst(&argument, 1);
    check(inst.is_ok(), "generic mixed ABI arguments");
    auto closed = vm::GenericClass::get_class(vm::Class::get_type_def_gid(generic_def.unwrap()), inst.unwrap());
    check(closed.is_ok() && vm::Class::initialize_all(closed.unwrap()).is_ok(), "generic mixed ABI class");
    auto object = LEANCLR_NEWOBJ(closed.unwrap(), gc::GcAllocSite::make_internal(__FILE__, __LINE__, "generic mixed integer ABI"));
    check(object.is_ok(), "generic integer reader allocation");
    auto* target = const_cast<metadata::RtMethodInfo*>(vm::Method::find_matched_method_in_class_by_name(closed.unwrap(), "Read"));
    const auto original = *target;
    const auto* bridge = vm::Method::find_matched_method_in_class_by_name(fixture, "CallReader");
    const void* args[] = {object.unwrap()};
    for (uint8_t flags = 0; flags < 4; ++flags)
    {
        target->invoker_type = metadata::RtInvokerType::Aot;
        target->aot_return_protocol = flags;
        target->method_ptr = target->virtual_method_ptr = (flags & 2) ? reinterpret_cast<metadata::RtManagedMethodPointer>(packed_generic_integer)
                                                                      : reinterpret_cast<metadata::RtManagedMethodPointer>(legacy_generic_integer);
        injected_error = false;
        check(invoke_int(bridge, args) == INT32_MIN, "generic target ABI/context");
        injected_error = true;
        auto failure = vm::Runtime::invoke_with_run_cctor(bridge, nullptr, args);
        check(failure.is_err() && failure.unwrap_err() == RtErr::ManagedException && vm::Exception::get_current_exception() &&
                  reinterpret_cast<vm::RtObject*>(vm::Exception::get_current_exception())->klass == vm::Class::get_corlib_types().cls_out_of_memory_exception,
              "generic mixed integer OOM");
    }
    *target = original;
    injected_error = false;
}

int main(int argc, char** argv)
{
    configure_test_assertions();
    check(argc >= 2, "DLL directory required");
    dlls = argv[1];
    const bool expected_aot = argc < 3 || std::strcmp(argv[2], "interpreted") != 0;
    if (!host::Services::set_file_loader(load))
        leanclr::panic("Configure file loader before host services are used");
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    check(vm::Runtime::initialize().is_ok(), "initialize runtime");
    // A boxed virtual call into an interpreted value type must unbox exactly
    // once, including Release builds with identical-code folding enabled.
    auto* int_class = vm::Class::get_corlib_types().cls_int32;
    check(vm::Class::initialize_all(int_class).is_ok(), "initialize integer formatting");
    const auto* format_integer = vm::Method::find_matched_method_in_class_by_name_and_param_count(int_class, "ToString", 0);
    check(format_integer != nullptr, "integer formatter");
    int32_t integer_value = 42;
    auto boxed_integer = LEANCLR_BOX_OBJECT_INTERNAL(int_class, &integer_value, "boxed virtual formatting regression");
    check(boxed_integer.is_ok(), "box integer formatter receiver");
    interp::RtStackObject format_args[1]{}, format_ret[1]{};
    format_args[0].obj = boxed_integer.unwrap();
    check(vm::Runtime::virtual_invoke_stackobject_arguments_without_run_cctor(format_integer, format_args, format_ret).is_ok(), "boxed integer formatting");
    auto* formatted = reinterpret_cast<vm::RtString*>(format_ret[0].obj);
    check(formatted && formatted->length == 2 && vm::String::get_chars_ptr(formatted)[0] == '4' && vm::String::get_chars_ptr(formatted)[1] == '2',
          "boxed virtual call read object header instead of integer payload");
    auto assembly = vm::Assembly::load_by_name("GenericFixtures");
    check(assembly.is_ok(), "load fixtures");
    auto type = assembly.unwrap()->mod->get_class_by_name("GenericFixtures", false, false);
    check(type.is_ok() && type.unwrap(), "fixture class");
    auto* klass = type.unwrap();
    check(vm::Class::initialize_all(klass).is_ok(), "initialize fixture type");
    auto* run = vm::Method::find_matched_method_in_class_by_name(klass, "Run");
    const int code = invoke_int(run, nullptr);
    if (code)
        std::fprintf(stderr, "Fixture failure code: %d\n", code);
    check(code == 0, "managed fixture behavior");
    integer_bridge_checks(klass);
    // Drive the generated interface bridge against both actual return ABIs,
    // independently of the caller's generation mode (also catches Wasm traps).
    auto reader_type = vm::Class::find_nested_class_by_name(klass, "ReferenceReader", false);
    check(reader_type.is_ok() && reader_type.unwrap(), "reference reader class");
    check(vm::Class::initialize_all(reader_type.unwrap()).is_ok(), "initialize reference reader");
    auto reader = LEANCLR_NEWOBJ(reader_type.unwrap(), gc::GcAllocSite::make_internal(__FILE__, __LINE__, "return protocol test"));
    check(reader.is_ok(), "allocate reference reader");
    const void* reader_args[] = {reader.unwrap()};
    auto* get = const_cast<metadata::RtMethodInfo*>(vm::Method::find_matched_method_in_class_by_name(reader_type.unwrap(), "Get"));
    const auto original_protocol = get->aot_return_protocol;
    const auto original_method = get->method_ptr;
    const auto original_virtual = get->virtual_method_ptr;
    auto* touch = const_cast<metadata::RtMethodInfo*>(vm::Method::find_matched_method_in_class_by_name(reader_type.unwrap(), "Touch"));
    const auto original_touch = touch->method_ptr;
    const auto original_touch_virtual = touch->virtual_method_ptr;
    const auto original_touch_protocol = touch->aot_return_protocol;
    const auto* bridge = vm::Method::find_matched_method_in_class_by_name(klass, "ReferenceBridge");
    const auto* void_bridge = vm::Method::find_matched_method_in_class_by_name(klass, "VoidBridge");
    for (uint32_t protocol = 0; protocol < 4; ++protocol)
    {
        get->aot_return_protocol = touch->aot_return_protocol = static_cast<uint8_t>(protocol);
        get->method_ptr = get->virtual_method_ptr = (protocol & 1) ? reinterpret_cast<metadata::RtManagedMethodPointer>(scalar_reference)
                                                                   : reinterpret_cast<metadata::RtManagedMethodPointer>(legacy_reference);
        touch->method_ptr = touch->virtual_method_ptr =
            (protocol & 1) ? reinterpret_cast<metadata::RtManagedMethodPointer>(scalar_void) : reinterpret_cast<metadata::RtManagedMethodPointer>(legacy_void);
        injected_error = false;
        auto ok = vm::Runtime::invoke_with_run_cctor(bridge, nullptr, reader_args);
        check(ok.is_ok() && ok.unwrap() == nullptr, "mixed return ABI successful null");
        auto void_ok = vm::Runtime::invoke_with_run_cctor(void_bridge, nullptr, reader_args);
        check(void_ok.is_ok(), "mixed return ABI void success");
        injected_error = true;
        auto error = vm::Runtime::invoke_with_run_cctor(bridge, nullptr, reader_args);
        check(error.is_err(), "mixed return ABI OOM propagation");
        auto void_error = vm::Runtime::invoke_with_run_cctor(void_bridge, nullptr, reader_args);
        check(void_error.is_err(), "mixed return ABI void OOM propagation");
    }
    injected_error = false;
    get->method_ptr = original_method;
    get->virtual_method_ptr = original_virtual;
    touch->method_ptr = original_touch;
    touch->virtual_method_ptr = original_touch_virtual;
    get->aot_return_protocol = original_protocol;
    touch->aot_return_protocol = original_touch_protocol;
    const auto* null_method = vm::Method::find_matched_method_in_class_by_name(klass, "NullReferenceResult");
    auto null_result = vm::Runtime::invoke_with_run_cctor(null_method, nullptr, nullptr);
    check(null_result.is_ok() && null_result.unwrap() == nullptr, "successful null reference through AOT invoker");
    const auto* void_method = vm::Method::find_matched_method_in_class_by_name(klass, "VoidResult");
    int divisor = 3;
    const void* void_args[] = {&divisor};
    auto void_ok = vm::Runtime::invoke_with_run_cctor(void_method, nullptr, void_args);
    check(void_ok.is_ok(), "void success through AOT invoker");
    divisor = 0;
    auto void_error = vm::Runtime::invoke_with_run_cctor(void_method, nullptr, void_args);
    check(void_error.is_err(), "void exception through AOT invoker");
    const auto& corlib = vm::Class::get_corlib_types();
    auto* identity = inflate(klass, "Identity", corlib.cls_int32->by_val);
    check((identity->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "identity invocation must use expected backend");
    int32_t value = 73;
    const void* args[] = {&value};
    check(invoke_int(identity, args) == 73, "reflective/interop identity invocation");
    auto* text = vm::String::create_string_from_utf8cstr("reference return");
    check(text != nullptr, "allocate string argument");
    const void* string_args[] = {text};
    auto reference = vm::Runtime::invoke_with_run_cctor(inflate(klass, "Identity", corlib.cls_string->by_val), nullptr, string_args);
    check(reference.is_ok() && reference.unwrap() == reinterpret_cast<vm::RtObject*>(text), "closed generic reference invoker");
    auto* no_int = inflate(klass, "NoArguments", corlib.cls_int32->by_val);
    auto* no_string = inflate(klass, "NoArguments", corlib.cls_string->by_val);
    check((no_int->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "no-args registration");
    check(invoke_int(no_int, nullptr) == 11 && invoke_int(no_string, nullptr) == 22, "distinct same-signature generic instances");
    auto* absent = inflate(klass, "Identity", corlib.cls_double->by_val);
    check(absent->invoker_type != metadata::RtInvokerType::Aot, "unregistered instance must not borrow another specialization");
    auto holder_def = assembly.unwrap()->mod->get_class_by_name("GenericHolder`1", false, false);
    check(holder_def.is_ok() && holder_def.unwrap(), "generic class definition");
    const metadata::RtTypeSig* holder_arg = corlib.cls_int32->by_val;
    auto holder_inst = metadata::MetadataCache::get_pooled_generic_inst(&holder_arg, 1);
    check(holder_inst.is_ok(), "generic class arguments");
    auto holder = vm::GenericClass::get_class(vm::Class::get_type_def_gid(holder_def.unwrap()), holder_inst.unwrap());
    check(holder.is_ok() && vm::Class::initialize_all(holder.unwrap()).is_ok(), "closed generic class");
    auto* forward = vm::Method::find_matched_method_in_class_by_name(holder.unwrap(), "Forward");
    check(forward && ((forward->invoker_type == metadata::RtInvokerType::Aot) == expected_aot), "generic class method registration");
    check(invoke_int(forward, args) == 73, "dynamic invocation on generic class");
    auto library = vm::Assembly::load_by_name("GenericLibrary");
    check(library.is_ok(), "load generic library");
    check(library.unwrap()->mod->get_aot_module_data() == nullptr, "generic-only dependency must not need a per-module AOT table");
    auto library_type = library.unwrap()->mod->get_class_by_name("GenericLibrary", false, false);
    check(library_type.is_ok() && vm::Class::initialize_all(library_type.unwrap()).is_ok(), "generic library type");
    auto* library_identity = inflate(library_type.unwrap(), "Identity", corlib.cls_int32->by_val);
    check((library_identity->invoker_type == metadata::RtInvokerType::Aot) == expected_aot, "cross-module specialization registration");
    check(invoke_int(library_identity, args) == 73, "cross-module generic invocation");
    auto* divide = inflate(klass, "Divide", corlib.cls_int32->by_val);
    value = 0;
    auto error = vm::Runtime::invoke_with_run_cctor(divide, nullptr, args);
    check(error.is_err(), "AOT generic exception must propagate");
    std::puts("Generic AOT end-to-end checks passed");
}
