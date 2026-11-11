#include "public/engine_host.h"
#include "alloc/general_allocation.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/rt_exception.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace leanclr;

struct LeanclrEngineSession
{
    std::atomic<LeanclrEngineState> state{LEANCLR_ENGINE_EMPTY};
    LeanclrEngineConfig config{};
    char assembly[256]{};
    char type[512]{};
    uint64_t owner = 0;
    bool busy = false;
    LeanclrEngineStatus loader_error = LEANCLR_ENGINE_OK;
    const metadata::RtMethodInfo* tick = nullptr;
    const metadata::RtMethodInfo* stop = nullptr;
};

namespace
{
LeanclrEngineSession session;

LeanclrEngineStatus report(LeanclrEngineError* out, LeanclrEngineStatus status, const char* message, RtErr error = RtErr::None)
{
    if (out)
    {
        out->status = status;
        out->runtime_error = static_cast<uint32_t>(error);
        std::snprintf(out->message, sizeof(out->message), "%s", message);
    }
    return status;
}

LeanclrEngineStatus runtime_error(LeanclrEngineError* out, RtErr error, const char* phase)
{
    if (error == RtErr::ManagedException)
        vm::Exception::get_and_clear_current_exception();
    const auto status = error == RtErr::ManagedException ? LEANCLR_ENGINE_MANAGED_EXCEPTION
                        : error == RtErr::OutOfMemory    ? LEANCLR_ENGINE_OUT_OF_MEMORY
                                                         : LEANCLR_ENGINE_RUNTIME_FAILED;
    return report(out, status, phase, error);
}

bool load_file(const char* name, const char* extension, vm::FileData& out)
{
    LeanclrEngineFile file{};
    const int32_t found = session.config.load_file(session.config.context, name, extension, &file);
    if (found <= 0)
    {
        if (found < 0)
            session.loader_error = LEANCLR_ENGINE_LOAD_FAILED;
        return false;
    }
    bool ok = false;
    if (!file.data || file.length == 0)
        session.loader_error = LEANCLR_ENGINE_LOAD_FAILED;
    else if (file.length > session.config.max_assembly_bytes)
        session.loader_error = LEANCLR_ENGINE_LIMIT_EXCEEDED;
    else
    {
        auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(file.length));
        if (bytes)
        {
            std::memcpy(bytes, file.data, file.length);
            out = {bytes, file.length, false};
            ok = true;
        }
        else
            session.loader_error = LEANCLR_ENGINE_OUT_OF_MEMORY;
    }
    session.config.release_file(session.config.context, &file);
    return ok;
}

const metadata::RtMethodInfo* find_entry(const metadata::RtClass* klass, const char* name, bool tick)
{
    const metadata::RtMethodInfo* result = nullptr;
    for (uint16_t i = 0; i < klass->method_count; ++i)
    {
        const auto* method = klass->methods[i];
        if (std::strcmp(method->name, name) != 0)
            continue;
        // Ambiguous/invalid overloads are rejected at startup, never resolved per tick.
        if (result || !vm::Method::is_static(method) || method->generic_container || method->generic_method || method->return_type->is_by_ref() ||
            !method->return_type->is_void() || method->parameter_count != (tick ? 1 : 0))
            return nullptr;
        if (tick && (method->parameters[0]->is_by_ref() || method->parameters[0]->ele_type != metadata::RtElementType::R4))
            return nullptr;
        const auto row = klass->image->get_cli_image().read_method(metadata::RtToken::decode_rid(method->token));
        if (!row)
            return nullptr;
        auto signature = klass->image->get_decoded_blob_reader(row->signature);
        if (signature.is_err())
            return nullptr;
        auto reader = signature.unwrap();
        uint8_t convention;
        if (!reader.try_read_byte(convention) || convention != static_cast<uint8_t>(metadata::RtSigType::Default))
            return nullptr;
        result = method;
    }
    return result;
}

LeanclrEngineStatus check_owner(LeanclrEngineSession* value, LeanclrEngineError* error)
{
    if (value != &session)
        return report(error, LEANCLR_ENGINE_INVALID_ARGUMENT, "Unknown engine session");
    auto state = session.state.load(std::memory_order_acquire);
    if (state == LEANCLR_ENGINE_EMPTY)
        return report(error, LEANCLR_ENGINE_INVALID_STATE, "Engine has not started");
    // Terminal operations do not access callbacks whose owner may have been freed.
    if (state == LEANCLR_ENGINE_STOPPED || state == LEANCLR_ENGINE_FAILED)
        return report(error, LEANCLR_ENGINE_INVALID_STATE, "Engine is terminal");
    if (state != LEANCLR_ENGINE_RUNNING)
        return report(error, LEANCLR_ENGINE_REENTRANT, "Runtime operation is already active");
    if (session.config.current_thread_id(session.config.context) != session.owner)
        return report(error, LEANCLR_ENGINE_WRONG_THREAD, "Runtime owner thread required");
    if (session.busy)
        return report(error, LEANCLR_ENGINE_REENTRANT, "Runtime operation is already active");
    return LEANCLR_ENGINE_OK;
}
} // namespace

LeanclrEngineStatus leanclr_engine_start(const LeanclrEngineConfig* config, LeanclrEngineSession** output, LeanclrEngineError* error)
{
    if (output)
        *output = nullptr;
    if (session.state.load(std::memory_order_acquire) != LEANCLR_ENGINE_EMPTY)
        return report(error, LEANCLR_ENGINE_INVALID_STATE, "Engine initialization is process scoped");
    if (!config || !output || config->size != sizeof(LeanclrEngineConfig))
        return report(error, LEANCLR_ENGINE_INVALID_ARGUMENT, "Invalid engine configuration size");
    if (config->abi_version != LEANCLR_ENGINE_ABI_VERSION)
        return report(error, LEANCLR_ENGINE_UNSUPPORTED_VERSION, "Unsupported engine ABI version");
    if (!config->load_file || !config->release_file || !config->current_thread_id || !config->assembly_name || !config->entry_type ||
        !config->assembly_name[0] || !config->entry_type[0] || std::strlen(config->assembly_name) >= sizeof(session.assembly) ||
        std::strlen(config->entry_type) >= sizeof(session.type))
        return report(error, LEANCLR_ENGINE_INVALID_ARGUMENT, "Missing callbacks or invalid entry names");
#if !LEANCLR_BCL_CORECLR_NET10
    if (config->max_type_handles)
        return report(error, LEANCLR_ENGINE_UNSUPPORTED, "Type facade budget requires the net10 profile");
#endif
    const auto owner = config->current_thread_id(config->context);
    if (!owner)
        return report(error, LEANCLR_ENGINE_INVALID_ARGUMENT, "Thread identity must be nonzero");
    auto expected = LEANCLR_ENGINE_EMPTY;
    if (!session.state.compare_exchange_strong(expected, LEANCLR_ENGINE_STARTING, std::memory_order_acq_rel))
        return report(error, LEANCLR_ENGINE_INVALID_STATE, "Engine initialization is process scoped");
    session.config = *config;
    std::strcpy(session.assembly, config->assembly_name);
    std::strcpy(session.type, config->entry_type);
    session.config.assembly_name = session.assembly;
    session.config.entry_type = session.type;
    if (!session.config.max_assembly_bytes)
        session.config.max_assembly_bytes = 64 * 1024 * 1024;
    session.owner = owner;
    *output = &session;
    auto fail = [&](LeanclrEngineStatus status)
    {
        vm::Exception::get_and_clear_current_exception();
        vm::Runtime::shutdown();
        session.state.store(LEANCLR_ENGINE_FAILED, std::memory_order_release);
        return status;
    };
    if (vm::Runtime::get_state() != vm::Runtime::State::Uninitialized)
    {
        // Never shut down a runtime owned by another embedding API.
        session.state.store(LEANCLR_ENGINE_FAILED, std::memory_order_release);
        return report(error, LEANCLR_ENGINE_INVALID_STATE, "Runtime is already owned");
    }
    vm::Settings::set_file_loader(load_file);
    vm::Settings::set_aot_modules_data(static_cast<const metadata::RtAotModulesData*>(config->aot_modules));
#if LEANCLR_BCL_CORECLR_NET10
    if (config->max_type_handles)
        vm::Settings::set_max_runtime_type_handles(config->max_type_handles);
#endif
    auto initialized = vm::Runtime::initialize();
    if (initialized.is_err())
    {
        const auto failure = initialized.unwrap_err();
        if (session.loader_error != LEANCLR_ENGINE_OK)
            return fail(report(error, session.loader_error, "CoreLib loading failed", failure));
        return fail(runtime_error(error, failure, "Runtime initialization failed"));
    }
    auto assembly = vm::Assembly::load_by_name(session.assembly);
    if (assembly.is_err())
    {
        const auto failure = assembly.unwrap_err();
        return fail(report(error, session.loader_error == LEANCLR_ENGINE_OK ? LEANCLR_ENGINE_LOAD_FAILED : session.loader_error,
                           "Entry assembly loading failed", failure));
    }
    auto klass = assembly.unwrap()->mod->get_class_by_name(session.type, false, true);
    if (klass.is_err())
        return fail(runtime_error(error, klass.unwrap_err(), "Entry type resolution failed"));
    if (klass.unwrap()->generic_container)
        return fail(report(error, LEANCLR_ENGINE_METHOD_CONTRACT, "Engine entry type must not be an open generic"));
    auto methods = vm::Class::initialize_methods(klass.unwrap());
    if (methods.is_err())
        return fail(runtime_error(error, methods.unwrap_err(), "Entry methods resolution failed"));
    const auto* start = find_entry(klass.unwrap(), "Initialize", false);
    session.tick = find_entry(klass.unwrap(), "Tick", true);
    session.stop = find_entry(klass.unwrap(), "Shutdown", false);
    if (!start || !session.tick || !session.stop)
        return fail(report(error, LEANCLR_ENGINE_METHOD_CONTRACT, "Expected static void Initialize(), Tick(float), Shutdown()"));
    auto started = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(start, nullptr, nullptr);
    if (started.is_err())
        return fail(runtime_error(error, started.unwrap_err(), "Managed Initialize failed"));
    session.state.store(LEANCLR_ENGINE_RUNNING, std::memory_order_release);
    return report(error, LEANCLR_ENGINE_OK, "");
}

LeanclrEngineStatus leanclr_engine_tick(LeanclrEngineSession* value, float delta, LeanclrEngineError* error)
{
    auto checked = check_owner(value, error);
    if (checked != LEANCLR_ENGINE_OK)
        return checked;
    if (!std::isfinite(delta))
        return report(error, LEANCLR_ENGINE_INVALID_ARGUMENT, "Tick delta must be finite");
    session.busy = true;
    interp::RtStackObject argument{};
    argument.f32 = delta;
    auto result = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(session.tick, &argument, nullptr);
    session.busy = false;
    if (result.is_err())
        return runtime_error(error, result.unwrap_err(), "Managed Tick failed");
    return report(error, LEANCLR_ENGINE_OK, "");
}

LeanclrEngineStatus leanclr_engine_stop(LeanclrEngineSession* value, LeanclrEngineError* error)
{
    if (value == &session)
    {
        auto state = session.state.load(std::memory_order_acquire);
        if (state == LEANCLR_ENGINE_STOPPED || state == LEANCLR_ENGINE_FAILED)
            return report(error, LEANCLR_ENGINE_OK, "");
    }
    auto checked = check_owner(value, error);
    if (checked != LEANCLR_ENGINE_OK)
        return checked;
    session.state.store(LEANCLR_ENGINE_STOPPING, std::memory_order_release);
    auto stopped = vm::Runtime::invoke_stackobject_arguments_with_run_cctor(session.stop, nullptr, nullptr);
    vm::Runtime::shutdown();
    session.state.store(LEANCLR_ENGINE_STOPPED, std::memory_order_release);
    if (stopped.is_err())
        return runtime_error(error, stopped.unwrap_err(), "Managed Shutdown failed; runtime is terminal");
    return report(error, LEANCLR_ENGINE_OK, "");
}

LeanclrEngineState leanclr_engine_state(const LeanclrEngineSession* value)
{
    return value == &session ? session.state.load(std::memory_order_acquire) : LEANCLR_ENGINE_EMPTY;
}
