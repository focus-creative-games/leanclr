#include "runtime.h"

#include "intrinsics.h"
#include "internal_calls.h"
#include "pinvoke.h"

#include "class.h"
#include "assembly.h"
#include "rt_string.h"
#include "rt_array.h"
#include "array_class.h"
#include "rt_exception.h"
#include "delegate.h"
#include "rt_thread.h"
#include "appdomain.h"
#include "method.h"
#include "object.h"
#include "environment.h"
#include "settings.h"

#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "metadata/aot_module.h"
#include "alloc/general_allocation.h"
#include "alloc/metadata_allocation.h"
#include "gc/garbage_collector.h"
#include "interp/machine_state.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace vm
{

// Helper structure for managing temporary buffers during method invocation
struct ScopeBufferGuard
{
    static constexpr size_t kTempArgsBufferSize = 16;
    static constexpr size_t kTempRetBufferSize = 4;

    interp::RtStackObject* args;
    interp::RtStackObject* ret;
    size_t args_size;
    size_t ret_size;
    interp::RtStackObject args_temp_buffer[kTempArgsBufferSize];
    interp::RtStackObject ret_temp_buffer[kTempRetBufferSize];
    utils::Vector<void*> temp_byref_valuetype_args_buffer;

    ScopeBufferGuard() : args(nullptr), ret(nullptr), args_size(0), ret_size(0)
    {
    }

    ~ScopeBufferGuard()
    {
        // Free args buffer if allocated
        if (args_size > kTempArgsBufferSize && args != nullptr)
        {
            alloc::GeneralAllocation::free(args);
        }

        // Free ret buffer if allocated
        if (ret_size > kTempRetBufferSize && ret != nullptr)
        {
            alloc::GeneralAllocation::free(ret);
        }

        // Free temporary byref value type buffers
        for (size_t i = 0; i < temp_byref_valuetype_args_buffer.size(); ++i)
        {
            alloc::GeneralAllocation::free(temp_byref_valuetype_args_buffer[i]);
        }
    }

    void* alloc_zeroed_temp_value_type_buffer(size_t size)
    {
        void* buffer = alloc::GeneralAllocation::malloc_zeroed(size);
        temp_byref_valuetype_args_buffer.push_back(buffer);
        return buffer;
    }

    struct ArgsAndRetBuffers
    {
        interp::RtStackObject* args_buffer;
        interp::RtStackObject* ret_buffer;
        ArgsAndRetBuffers(interp::RtStackObject* args_buf, interp::RtStackObject* ret_buf) : args_buffer(args_buf), ret_buffer(ret_buf)
        {
        }
    };

    RtResult<ArgsAndRetBuffers> prepare_args_and_ret_buffer(const metadata::RtMethodInfo* method)
    {
        size_t total_args_size = method->total_arg_stack_object_size;
        size_t ret_size_val = method->ret_stack_object_size;

        interp::RtStackObject* args_buffer;
        if (total_args_size <= kTempArgsBufferSize)
        {
            args_buffer = args_temp_buffer;
            std::memset(args_buffer, 0, total_args_size * sizeof(interp::RtStackObject));
        }
        else
        {
            args_buffer = new interp::RtStackObject[total_args_size];
            std::memset(args_buffer, 0, total_args_size * sizeof(interp::RtStackObject));
        }
        args_size = total_args_size;
        args = args_buffer;

        interp::RtStackObject* ret_buffer;
        if (ret_size_val <= kTempRetBufferSize)
        {
            ret_buffer = ret_temp_buffer;
            std::memset(ret_buffer, 0, ret_size_val * sizeof(interp::RtStackObject));
        }
        else
        {
            ret_buffer = new interp::RtStackObject[ret_size_val];
            std::memset(ret_buffer, 0, ret_size_val * sizeof(interp::RtStackObject));
        }
        ret_size = ret_size_val;
        ret = ret_buffer;

        RET_OK(ArgsAndRetBuffers(args_buffer, ret_buffer));
    }

    RtResult<ArgsAndRetBuffers> convert_params_to_temp_buffer(const metadata::RtMethodInfo* method, RtObject* obj, const void* const* params)
    {
        if (Method::get_param_count_include_this(method) == 0 && Method::is_void_return(method))
        {
            RET_OK(ArgsAndRetBuffers(nullptr, nullptr));
        }
        auto retBuffers = prepare_args_and_ret_buffer(method);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(ArgsAndRetBuffers, buffers, retBuffers);
        interp::RtStackObject* args_buffer = buffers.args_buffer;
        interp::RtStackObject* ret_buffer = buffers.ret_buffer;

        size_t dst_idx = 0;

        // Handle 'this' parameter for instance methods
        if (Method::is_instance(method))
        {
            RtObject* adjust_this;
            if (Class::is_value_type(method->parent))
            {
                adjust_this = obj + 1;
            }
            else
            {
                adjust_this = obj;
            }
            args_buffer[dst_idx].obj = adjust_this;
            dst_idx++;
        }

        // Handle method parameters
        for (size_t i = 0; i < Method::get_param_count_exclude_this(method); ++i)
        {
            const metadata::RtTypeSig* param_type_sig = method->parameters[i];
            const void* param = params[i];
            interp::RtStackObject& dst_arg = args_buffer[dst_idx];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, reduceTypeAndSize,
                                                    interp::InterpDefs::get_reduce_type_and_size_by_typesig(param_type_sig));
            switch (reduceTypeAndSize.reduce_type)
            {
            case metadata::RtArgOrLocOrFieldReduceType::I1:
                dst_arg.i32 = *reinterpret_cast<const int8_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::U1:
                dst_arg.i32 = *reinterpret_cast<const uint8_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::I2:
                dst_arg.i32 = *reinterpret_cast<const int16_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::U2:
                dst_arg.i32 = *reinterpret_cast<const uint16_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::I4:
            case metadata::RtArgOrLocOrFieldReduceType::R4:
                dst_arg.i32 = *reinterpret_cast<const int32_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::I8:
            case metadata::RtArgOrLocOrFieldReduceType::R8:
                dst_arg.i64 = *reinterpret_cast<const int64_t*>(param);
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::I:
            {
                if (param_type_sig->by_ref)
                {
                    dst_arg.cptr = param;
                }
                else
                {
                    dst_arg.cptr = *(const void**)(param);
                }
                dst_idx++;
                break;
            }
            case metadata::RtArgOrLocOrFieldReduceType::Ref:
                dst_arg.cptr = param;
                dst_idx++;
                break;
            case metadata::RtArgOrLocOrFieldReduceType::Other:
            {
                size_t stackObjectSize = interp::InterpDefs::get_stack_object_size_by_byte_size(reduceTypeAndSize.byte_size);
                std::memcpy(&dst_arg, param, stackObjectSize * sizeof(interp::RtStackObject));
                dst_idx += stackObjectSize;
                break;
            }
            default:
                RET_ASSERT_ERR(RtErr::ExecutionEngine);
            }
        }

        RET_OK(ArgsAndRetBuffers(args_buffer, ret_buffer));
    }

    RtResult<ArgsAndRetBuffers> convert_object_params_to_temp_buffer(const metadata::RtMethodInfo* method, RtObject* obj, RtObject** params, int32_t paramCount)
    {
        if (Method::get_param_count_include_this(method) == 0 && Method::is_void_return(method))
        {
            RET_OK(ArgsAndRetBuffers(nullptr, nullptr));
        }

        auto retBuffers = prepare_args_and_ret_buffer(method);
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(ArgsAndRetBuffers, buffers, retBuffers);
        interp::RtStackObject* args_buffer = buffers.args_buffer;
        interp::RtStackObject* ret_buffer = buffers.ret_buffer;

        size_t dst_idx = 0;

        // Handle 'this' parameter for instance methods
        if (Method::is_instance(method))
        {
            RtObject* this_ptr;
            if (Class::is_value_type(method->parent))
            {
                this_ptr = obj + 1;
            }
            else
            {
                this_ptr = obj;
            }
            args_buffer[dst_idx].obj = this_ptr;
            dst_idx++;
        }

        size_t method_param_count = Method::get_param_count_exclude_this(method);
        if (method_param_count == 0)
        {
            RET_OK(ArgsAndRetBuffers(args_buffer, ret_buffer));
        }

        for (size_t i = 0; i < method_param_count; ++i)
        {
            const metadata::RtTypeSig* param_type_sig = method->parameters[i];
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, param_klass, Class::get_class_from_typesig(param_type_sig));
            RET_ERR_ON_FAIL(Class::initialize_all(param_klass));
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(interp::ReduceTypeAndSize, reduceTypeAndSize,
                                                    interp::InterpDefs::get_reduce_type_and_size_by_typesig(param_type_sig));

            RtObject* param = params[i];
            interp::RtStackObject& dst = args_buffer[dst_idx];
            dst_idx += interp::InterpDefs::get_stack_object_size_by_byte_size(reduceTypeAndSize.byte_size);

            if (param_type_sig->by_ref)
            {
                if (Class::is_value_type(param_klass))
                {
                    void* buffer = alloc_zeroed_temp_value_type_buffer(param_klass->instance_size_without_header);
                    RET_ERR_ON_FAIL(Object::unbox_any(param, param_klass, buffer, false));
                    dst.ptr = buffer;
                }
                else
                {
                    dst.ptr = &params[i];
                }
            }
            else
            {
                if (Class::is_value_type(param_klass))
                {
                    if (param != nullptr)
                    {
                        RET_ERR_ON_FAIL(Object::unbox_any(param, param_klass, reinterpret_cast<uint8_t*>(&dst), true));
                    }
                }
                else
                {
                    dst.ptr = param;
                }
            }
        }

        RET_OK(ArgsAndRetBuffers(args_buffer, ret_buffer));
    }
};

// Static local function: convert return value (simplified)
static RtResult<RtObject*> convert_return_value(const metadata::RtTypeSig* return_type, interp::RtStackObject* ret_buffer)
{
    if (return_type->is_void())
    {
        RET_OK(static_cast<RtObject*>(nullptr));
    }

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, return_klass, Class::get_class_from_typesig(return_type));
    if (Class::is_value_type(return_klass))
    {
        return Object::box_object(return_klass, ret_buffer);
    }
    else
    {
        if (return_type->by_ref)
        {
            const void* address = ret_buffer->ptr;
            if (Class::is_value_type(return_klass))
            {
                return Object::box_object(return_klass, address);
            }
            else
            {
                RET_OK(*(RtObject**)(address));
            }
        }
        else
        {
            RET_OK(*(RtObject**)(ret_buffer));
        }
    }
    RET_OK(ret_buffer->obj);
}

// Static local function: invoke without running static constructor
static RtResult<RtObject*> invoke_without_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, const void* const* params)
{
    ScopeBufferGuard guard;
    auto retBuffers = guard.convert_params_to_temp_buffer(method, obj, params);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(ScopeBufferGuard::ArgsAndRetBuffers, buffers, retBuffers);

    interp::RtStackObject* arg_buffer = buffers.args_buffer;
    interp::RtStackObject* ret_buffer = buffers.ret_buffer;

    // Invoke the method
    auto invoke_ptr = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(method->invoke_method_ptr);
    RET_ERR_ON_FAIL(invoke_ptr(method->method_ptr, method, arg_buffer, ret_buffer));

    return convert_return_value(method->return_type, ret_buffer);
}

// Public Runtime functions implementation

RtResultVoid Runtime::initialize()
{
    // Initialize subsystems
    alloc::MetadataAllocation::init();
    Intrinsics::initialize();
    InternalCalls::initialize();
    PInvokes::initialize();

    auto aot_modules_data = Settings::get_aot_modules_data();
    if (aot_modules_data != nullptr)
    {
        metadata::AotModule::register_aot_modules(aot_modules_data);
    }

    auto internal_func_initializer = Settings::get_internal_functions_initializer();
    if (internal_func_initializer != nullptr)
    {
        internal_func_initializer();
    }

    metadata::MetadataCache::initialize();
    interp::MachineState::initialize();
    gc::GarbageCollector::initialize();

    RET_ERR_ON_FAIL(Assembly::load_corlib());
    RET_ERR_ON_FAIL(Class::initialize());
    RET_ERR_ON_FAIL(ArrayClass::initialize());
    RET_ERR_ON_FAIL(Class::verify_integrity_of_corlib_classes());
    RET_ERR_ON_FAIL(String::initialize());
    RET_ERR_ON_FAIL(Exception::initialize());
    RET_ERR_ON_FAIL(Delegate::initialize());
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtAppDomain*, defaultAppDomain, AppDomain::init_default_app_domain());
    Thread::attach_current_thread(defaultAppDomain);
    RET_ERR_ON_FAIL(AppDomain::initialize_context());

    int32_t argc;
    const char** argv;
    Settings::get_command_line_arguments(argc, argv);
    RET_ERR_ON_FAIL(Environment::init_cmdline_args(argv, argc));

    metadata::RtModuleDef* corlib_mod = Assembly::get_corlib()->mod;
    auto corlib_aot_module_data = corlib_mod->get_aot_module_data();
    if (corlib_aot_module_data != nullptr && corlib_aot_module_data->deferred_initializer)
    {
        corlib_aot_module_data->deferred_initializer(corlib_mod);
    }

    RET_VOID_OK();
}

void Runtime::shutdown()
{
    // todo: implement shutdown logic
}

RtResultVoid Runtime::run_class_static_constructor(const metadata::RtClass* klass)
{
    assert(klass);

    if (Class::is_cctor_not_finished(klass))
    {
        RET_ERR_ON_FAIL(run_module_static_constructor(klass->image));
        RET_ERR_ON_FAIL(Class::initialize_all(const_cast<metadata::RtClass*>(klass)));
        Class::set_cctor_finished(const_cast<metadata::RtClass*>(klass));

        const metadata::RtMethodInfo* cctor = Class::get_static_constructor(klass);
        if (cctor)
        {
            RET_ERR_ON_FAIL(invoke_without_run_cctor(cctor, nullptr, nullptr));
        }
    }

    RET_VOID_OK();
}

RtResult<const metadata::RtMethodInfo*> Runtime::get_module_constructor(metadata::RtModuleDef* module)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, module_klass, module->get_global_type_def());

    if (module_klass == nullptr)
    {
        RET_OK(static_cast<const metadata::RtMethodInfo*>(nullptr));
    }

    RET_ERR_ON_FAIL(Class::initialize_all(module_klass));

    const metadata::RtMethodInfo* cctor = Class::get_static_constructor(module_klass);
    RET_OK(cctor);
}

RtResultVoid Runtime::run_module_static_constructor(metadata::RtModuleDef* module)
{
    if (!module->is_module_cctor_finished())
    {
        module->set_module_cctor_finished();

        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, cctor, get_module_constructor(module));

        if (cctor != nullptr)
        {
            RET_ERR_ON_FAIL(invoke_without_run_cctor(cctor, nullptr, nullptr));
        }
    }
    RET_VOID_OK();
}

RtResult<RtObject*> Runtime::invoke_with_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, const void* const* params)
{
    assert(method);

    if (Method::is_static(method) && Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(run_class_static_constructor(method->parent));
    }

    return invoke_without_run_cctor(method, obj, params);
}

RtResult<RtObject*> Runtime::invoke_object_arguments_without_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, RtObject** params,
                                                                       int32_t paramCount)
{
    assert(method);

    const metadata::RtMethodInfo* actual_method = method;
    RtObject* actual_obj = obj;
    bool return_instance = Method::is_ctor(method) && obj == nullptr;

    if (return_instance)
    {
        assert(!Class::is_nullable_type(method->parent));
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtObject*, new_obj, Object::new_object(method->parent));
        actual_obj = new_obj;
    }

    if (Method::is_virtual(method))
    {
        if (actual_obj == nullptr)
        {
            RET_ERR(core::RtErr::NullReference);
        }
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtMethodInfo*, virt_method, Method::get_virtual_method_impl(actual_obj, method));
        actual_method = virt_method;
    }

    ScopeBufferGuard guard;
    auto retBuffers = guard.convert_object_params_to_temp_buffer(actual_method, actual_obj, params, paramCount);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(ScopeBufferGuard::ArgsAndRetBuffers, buffers, retBuffers);

    interp::RtStackObject* arg_buffer = buffers.args_buffer;
    interp::RtStackObject* ret_buffer = buffers.ret_buffer;

    // Invoke the method
    auto invoke_ptr = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(actual_method->invoke_method_ptr);
    RET_ERR_ON_FAIL(invoke_ptr(actual_method->method_ptr, actual_method, arg_buffer, ret_buffer));

    if (return_instance)
    {
        RET_OK(actual_obj);
    }
    else
    {
        return convert_return_value(actual_method->return_type, ret_buffer);
    }
}

RtResult<RtObject*> Runtime::invoke_object_arguments_with_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, RtObject** params, int32_t paramCount)
{
    assert(method);

    if (Method::is_static(method) && Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(run_class_static_constructor(method->parent));
    }

    return invoke_object_arguments_without_run_cctor(method, obj, params, paramCount);
}

RtResult<RtObject*> Runtime::invoke_array_arguments_without_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, RtArray* params)
{
    RtObject** params_data_start;
    int32_t paramCount;
    if (params)
    {
        params_data_start = Array::get_array_data_start_as<RtObject*>(params);
        paramCount = Array::get_array_length(params);
    }
    else
    {
        params_data_start = nullptr;
        paramCount = 0;
    }
    return invoke_object_arguments_without_run_cctor(method, obj, params_data_start, paramCount);
}

RtResult<RtObject*> Runtime::invoke_array_arguments_with_run_cctor(const metadata::RtMethodInfo* method, RtObject* obj, RtArray* params)
{
    if (Method::is_static(method) && Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(run_class_static_constructor(method->parent));
    }

    return invoke_array_arguments_without_run_cctor(method, obj, params);
}

RtResultVoid Runtime::invoke_stackobject_arguments_without_run_cctor(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                     interp::RtStackObject* ret) noexcept
{
    assert(method);

    auto invoke_ptr = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(method->invoke_method_ptr);
    return invoke_ptr(method->method_ptr, method, params, ret);
}

RtResultVoid Runtime::virtual_invoke_stackobject_arguments_without_run_cctor(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                             interp::RtStackObject* ret) noexcept
{
    assert(method);

    auto invoke_ptr = CAST_AS_NOEXCEP_INVOKE_METHOD_POINTER(method->virtual_invoke_method_ptr);
    return invoke_ptr(method->method_ptr, method, params, ret);
}

RtResultVoid Runtime::invoke_stackobject_arguments_with_run_cctor(const metadata::RtMethodInfo* method, const interp::RtStackObject* params,
                                                                  interp::RtStackObject* ret) noexcept
{
    assert(method);

    if (Method::is_static(method) && Class::is_cctor_not_finished(method->parent))
    {
        RET_ERR_ON_FAIL(run_class_static_constructor(method->parent));
    }

    return invoke_stackobject_arguments_without_run_cctor(method, params, ret);
}

} // namespace vm
} // namespace leanclr
