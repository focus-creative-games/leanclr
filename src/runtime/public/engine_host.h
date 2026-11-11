#pragma once
#include "export.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LEANCLR_ENGINE_ABI_VERSION 1u
    typedef struct LeanclrEngineSession LeanclrEngineSession;

    typedef enum LeanclrEngineStatus
    {
        LEANCLR_ENGINE_OK,
        LEANCLR_ENGINE_INVALID_ARGUMENT,
        LEANCLR_ENGINE_UNSUPPORTED_VERSION,
        LEANCLR_ENGINE_INVALID_STATE,
        LEANCLR_ENGINE_WRONG_THREAD,
        LEANCLR_ENGINE_REENTRANT,
        LEANCLR_ENGINE_LOAD_FAILED,
        LEANCLR_ENGINE_RUNTIME_FAILED,
        LEANCLR_ENGINE_MANAGED_EXCEPTION,
        LEANCLR_ENGINE_OUT_OF_MEMORY,
        LEANCLR_ENGINE_LIMIT_EXCEEDED,
        LEANCLR_ENGINE_METHOD_CONTRACT,
        LEANCLR_ENGINE_UNSUPPORTED
    } LeanclrEngineStatus;

    typedef enum LeanclrEngineState
    {
        LEANCLR_ENGINE_EMPTY,
        LEANCLR_ENGINE_STARTING,
        LEANCLR_ENGINE_RUNNING,
        LEANCLR_ENGINE_STOPPING,
        LEANCLR_ENGINE_STOPPED,
        LEANCLR_ENGINE_FAILED
    } LeanclrEngineState;

    /* Caller-owned, fixed storage. No managed exception or BCL object crosses this ABI. */
    typedef struct LeanclrEngineError
    {
        uint32_t status;
        uint32_t runtime_error;
        char message[256];
    } LeanclrEngineError;

    typedef struct LeanclrEngineFile
    {
        const uint8_t* data;
        size_t length;
        void* token;
    } LeanclrEngineFile;

    typedef struct LeanclrEngineConfig
    {
        uint32_t size;
        uint32_t abi_version;
        void* context;
        /* Positive: buffer found, released exactly once after copying. Zero: missing.
           Negative: host failure. Failed callbacks retain ownership of their data. */
        int32_t (*load_file)(void*, const char* name, const char* extension, LeanclrEngineFile*);
        void (*release_file)(void*, LeanclrEngineFile*);
        /* Must be stable, nonzero and distinct for each concurrently live thread. */
        uint64_t (*current_thread_id)(void*);
        const char* assembly_name;
        const char* entry_type;
        size_t max_assembly_bytes; /* zero selects 64 MiB */
        size_t max_type_handles;   /* zero keeps profile default; net10 only */
        const void* aot_modules;   /* optional registry emitted by this LeanAOT build */
    } LeanclrEngineConfig;

    /* One runtime owner and one initialization attempt per process. Config strings
       are copied. Callbacks/context remain alive through stop or failed start.
       Callbacks must not throw C++ exceptions. All managed calls run on start's thread.
       Managed entry contract: static void Initialize(), Tick(float), Shutdown(). */
    LEANCLR_API LeanclrEngineStatus leanclr_engine_start(const LeanclrEngineConfig*, LeanclrEngineSession**, LeanclrEngineError*);
    LEANCLR_API LeanclrEngineStatus leanclr_engine_tick(LeanclrEngineSession*, float delta_seconds, LeanclrEngineError*);
    /* Idempotent terminal quiescence, not unload/restart or automatic finalization. */
    LEANCLR_API LeanclrEngineStatus leanclr_engine_stop(LeanclrEngineSession*, LeanclrEngineError*);
    LEANCLR_API LeanclrEngineState leanclr_engine_state(const LeanclrEngineSession*);

#ifdef __cplusplus
}
#endif
