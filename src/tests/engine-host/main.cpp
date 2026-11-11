#include "engine_adapters.h"
#include "../aot-returns/test_assert.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#if !defined(__EMSCRIPTEN__)
#include <thread>
#endif

extern "C" int engine_header_contract(void);

static void check(bool value)
{
    if (!value)
        std::abort();
}

struct Context
{
    std::string runtime, managed;
    uint64_t thread = 1;
    size_t acquired = 0, released = 0;
    bool missing = false, reenter = false, fail_loader = false, fail_copy = false, forbid_callbacks = false;
    LeanclrEngineSession** session_slot = nullptr;
#if !defined(__EMSCRIPTEN__)
    std::thread::id native_owner = std::this_thread::get_id();
#endif
};

static uint64_t thread_id(void* context)
{
    auto* state = static_cast<Context*>(context);
    check(!state->forbid_callbacks);
#if !defined(__EMSCRIPTEN__)
    if (std::this_thread::get_id() != state->native_owner)
        return 2;
#endif
    return state->thread;
}

static int32_t load(void* context, const char* name, const char* extension, LeanclrEngineFile* output)
{
    auto& state = *static_cast<Context*>(context);
    if (state.reenter)
    {
        state.reenter = false;
        check(state.session_slot && *state.session_slot);
        check(leanclr_engine_tick(*state.session_slot, 0.25f, nullptr) == LEANCLR_ENGINE_REENTRANT);
        check(leanclr_engine_stop(*state.session_slot, nullptr) == LEANCLR_ENGINE_REENTRANT);
        LeanclrEngineConfig config{};
        LeanclrEngineSession* unused = nullptr;
        // Public handle remains unknown during the callback, but another start
        // with valid configuration must be rejected without changing its owner.
        config.size = sizeof(config);
        config.abi_version = LEANCLR_ENGINE_ABI_VERSION;
        config.context = context;
        config.current_thread_id = thread_id;
        config.load_file = load;
        config.release_file = [](void*, LeanclrEngineFile*) {};
        config.assembly_name = "ManagedNet10.EngineContracts";
        config.entry_type = "ManagedNet10.EngineContracts.Entry";
        check(leanclr_engine_start(&config, &unused, nullptr) == LEANCLR_ENGINE_INVALID_STATE && !unused);
    }
    if (state.fail_loader)
        return -1;
    if (state.missing)
        return 0;
    for (const auto& directory : {state.managed, state.runtime})
    {
        std::ifstream input(directory + "/" + name + "." + extension, std::ios::binary | std::ios::ate);
        if (!input)
            continue;
        const auto length = input.tellg();
        check(length > 0);
        auto* bytes = new std::vector<uint8_t>(static_cast<size_t>(length));
        input.seekg(0);
        check(static_cast<bool>(input.read(reinterpret_cast<char*>(bytes->data()), length)));
        *output = {bytes->data(), bytes->size(), bytes};
        ++state.acquired;
        if (state.fail_copy)
            output->length = std::numeric_limits<size_t>::max();
        return 1;
    }
    return 0;
}

static void release(void* context, LeanclrEngineFile* file)
{
    ++static_cast<Context*>(context)->released;
    delete static_cast<std::vector<uint8_t>*>(file->token);
}

int main(int argc, char** argv)
{
    configure_test_assertions();
    check(argc == 4 && engine_header_contract());
    const std::string mode = argv[1];
    Context context{argv[2], argv[3]};
    LeanclrEngineConfig config{};
    config.size = sizeof(config);
    config.abi_version = LEANCLR_ENGINE_ABI_VERSION;
    config.context = &context;
    config.load_file = load;
    config.release_file = release;
    config.current_thread_id = thread_id;
    config.assembly_name = "ManagedNet10.EngineContracts";
    config.entry_type = "ManagedNet10.EngineContracts.Entry";
    LeanclrEngineSession* session = nullptr;
    LeanclrEngineError error{};
    context.session_slot = &session;
    if (mode == "invalid" || mode == "retry")
    {
        config.abi_version = 999;
        check(leanclr_engine_start(&config, &session, &error) == LEANCLR_ENGINE_UNSUPPORTED_VERSION && !session);
        config.abi_version = LEANCLR_ENGINE_ABI_VERSION;
        config.size = 0;
        check(leanclr_engine_start(&config, &session, &error) == LEANCLR_ENGINE_INVALID_ARGUMENT && !session);
        config.size = sizeof(config);
        config.load_file = nullptr;
        check(leanclr_engine_start(&config, &session, &error) == LEANCLR_ENGINE_INVALID_ARGUMENT && !session);
        config.load_file = load;
        check(leanclr_engine_stop(reinterpret_cast<LeanclrEngineSession*>(uintptr_t{1}), &error) == LEANCLR_ENGINE_INVALID_ARGUMENT);
        if (mode == "invalid")
            return 0;
    }
    context.missing = mode == "missing";
    context.fail_loader = mode == "loader";
    context.fail_copy = mode == "oom";
    if (context.fail_copy)
        config.max_assembly_bytes = std::numeric_limits<size_t>::max();
    context.reenter = mode == "lifecycle";
    if (mode == "limit")
        config.max_assembly_bytes = 64;
    if (mode == "signature")
        config.entry_type = "ManagedNet10.EngineContracts.BadSignature";
    if (mode == "varargs")
        config.entry_type = "ManagedNet10.EngineContracts.VarargsEntry";
    if (mode == "generic")
        config.entry_type = "ManagedNet10.EngineContracts.GenericEntry`1";
    if (mode == "initialize")
        config.entry_type = "ManagedNet10.EngineContracts.InitializeThrows";
    if (mode == "shutdown")
        config.entry_type = "ManagedNet10.EngineContracts.ShutdownThrows";
    if (mode == "godot")
    {
        leanclr::host::GodotHostAdapter adapter;
        check(adapter.enter_tree(config, &error) == LEANCLR_ENGINE_OK);
        check(adapter.enter_tree(config, &error) == LEANCLR_ENGINE_INVALID_STATE);
        check(adapter.process(0.25f, &error) == LEANCLR_ENGINE_OK);
        check(adapter.process(0.25f, &error) == LEANCLR_ENGINE_OK);
        check(adapter.exit_tree(&error) == LEANCLR_ENGINE_OK);
        check(adapter.exit_tree(&error) == LEANCLR_ENGINE_OK);
        check(adapter.state() == LEANCLR_ENGINE_STOPPED);
    }
    else if (mode == "unreal")
    {
        leanclr::host::UnrealHostAdapter adapter;
        check(adapter.startup_module(config, &error) == LEANCLR_ENGINE_OK);
        check(adapter.tick_module(0.25f, &error) == LEANCLR_ENGINE_OK);
        check(adapter.tick_module(0.25f, &error) == LEANCLR_ENGINE_OK);
        check(adapter.shutdown_module(&error) == LEANCLR_ENGINE_OK);
        check(adapter.shutdown_module(&error) == LEANCLR_ENGINE_OK);
        check(adapter.startup_module(config, &error) == LEANCLR_ENGINE_INVALID_STATE);
    }
    else
    {
        const auto result = leanclr_engine_start(&config, &session, &error);
        if (mode == "missing" || mode == "limit" || mode == "signature" || mode == "initialize" || mode == "loader" || mode == "oom" || mode == "varargs" ||
            mode == "generic")
        {
            const bool contract_failure = mode == "signature" || mode == "varargs" || mode == "generic";
            const auto expected = mode == "oom"          ? LEANCLR_ENGINE_OUT_OF_MEMORY
                                  : mode == "loader"     ? LEANCLR_ENGINE_LOAD_FAILED
                                  : mode == "limit"      ? LEANCLR_ENGINE_LIMIT_EXCEEDED
                                  : contract_failure     ? LEANCLR_ENGINE_METHOD_CONTRACT
                                  : mode == "initialize" ? LEANCLR_ENGINE_MANAGED_EXCEPTION
                                                         : LEANCLR_ENGINE_RUNTIME_FAILED;
            if (result != expected)
                std::fprintf(stderr, "status=%d runtime=%u %s\n", result, error.runtime_error, error.message);
            check(result == expected && session && leanclr_engine_state(session) == LEANCLR_ENGINE_FAILED);
            check(leanclr_engine_stop(session, &error) == LEANCLR_ENGINE_OK);
            check(leanclr_engine_tick(session, 0.25f, &error) == LEANCLR_ENGINE_INVALID_STATE);
        }
        else
        {
            if (result != LEANCLR_ENGINE_OK)
                std::fprintf(stderr, "status=%d runtime=%u %s\n", result, error.runtime_error, error.message);
            check(result == LEANCLR_ENGINE_OK && leanclr_engine_state(session) == LEANCLR_ENGINE_RUNNING);
            if (mode != "shutdown")
            {
#if !defined(__EMSCRIPTEN__)
                std::thread wrong_thread(
                    [&]
                    {
                        LeanclrEngineError worker_error{};
                        check(leanclr_engine_tick(session, 0.25f, &worker_error) == LEANCLR_ENGINE_WRONG_THREAD);
                        check(leanclr_engine_stop(session, &worker_error) == LEANCLR_ENGINE_WRONG_THREAD);
                    });
                wrong_thread.join();
#endif
                context.thread = 2;
                check(leanclr_engine_tick(session, 0.25f, &error) == LEANCLR_ENGINE_WRONG_THREAD);
                check(leanclr_engine_stop(session, &error) == LEANCLR_ENGINE_WRONG_THREAD);
                context.thread = 1;
                check(leanclr_engine_tick(session, std::numeric_limits<float>::infinity(), &error) == LEANCLR_ENGINE_INVALID_ARGUMENT);
                check(leanclr_engine_tick(session, -1.f, &error) == LEANCLR_ENGINE_MANAGED_EXCEPTION && error.runtime_error != 0);
                check(leanclr_engine_tick(session, 0.25f, &error) == LEANCLR_ENGINE_OK);
                check(leanclr_engine_tick(session, 0.25f, &error) == LEANCLR_ENGINE_OK);
            }
            check(leanclr_engine_stop(session, &error) == (mode == "shutdown" ? LEANCLR_ENGINE_MANAGED_EXCEPTION : LEANCLR_ENGINE_OK));
            check(leanclr_engine_state(session) == LEANCLR_ENGINE_STOPPED);
            context.thread = 2;
            context.forbid_callbacks = true;
            check(leanclr_engine_stop(session, &error) == LEANCLR_ENGINE_OK);
            check(leanclr_engine_tick(session, 0.25f, &error) == LEANCLR_ENGINE_INVALID_STATE);
        }
        context.forbid_callbacks = true;
        check(leanclr_engine_start(&config, &session, &error) == LEANCLR_ENGINE_INVALID_STATE && !session);
    }
    check(context.acquired == context.released);
    std::puts("Engine host contract passed");
}
