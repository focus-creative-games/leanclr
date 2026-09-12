#ifndef LEANCLR_HOST_BRIDGE_H
#define LEANCLR_HOST_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LEANCLR_HOST_BRIDGE_ABI_VERSION 1u

typedef uint64_t LeanClrHostHandle;
typedef uint64_t LeanClrHostSubscription;

typedef enum LeanClrHostBridgeStatus
{
    LEANCLR_HOST_BRIDGE_OK = 0,
    LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT = 1,
    LEANCLR_HOST_BRIDGE_UNSUPPORTED_VERSION = 2,
    LEANCLR_HOST_BRIDGE_MISSING_CAPABILITY = 3,
    LEANCLR_HOST_BRIDGE_HOST_FAILURE = 4,
    LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED = 5,
    LEANCLR_HOST_BRIDGE_REENTRANT_CALL = 6,
    LEANCLR_HOST_BRIDGE_MANAGED_EXCEPTION = 7
} LeanClrHostBridgeStatus;

typedef enum LeanClrHostBridgeCapability
{
    LEANCLR_HOST_BRIDGE_CAP_LOGGING = 1ull << 0,
    LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY = 1ull << 1,
    LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY = 1ull << 2,
    LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH = 1ull << 3,
    LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK = 1ull << 4,
    LEANCLR_HOST_BRIDGE_CAP_VALUE_MARSHAL = 1ull << 5,
    LEANCLR_HOST_BRIDGE_CAP_DIAGNOSTICS = 1ull << 6
} LeanClrHostBridgeCapability;

typedef enum LeanClrHostBridgeLogLevel
{
    LEANCLR_HOST_BRIDGE_LOG_TRACE = 0,
    LEANCLR_HOST_BRIDGE_LOG_INFO = 1,
    LEANCLR_HOST_BRIDGE_LOG_WARNING = 2,
    LEANCLR_HOST_BRIDGE_LOG_ERROR = 3
} LeanClrHostBridgeLogLevel;

typedef enum LeanClrHostValueKind
{
    LEANCLR_HOST_VALUE_NULL = 0,
    LEANCLR_HOST_VALUE_BOOL = 1,
    LEANCLR_HOST_VALUE_INT32 = 2,
    LEANCLR_HOST_VALUE_FLOAT32 = 3,
    LEANCLR_HOST_VALUE_FLOAT64 = 4,
    LEANCLR_HOST_VALUE_STRING = 5,
    LEANCLR_HOST_VALUE_HANDLE = 6,
    LEANCLR_HOST_VALUE_VECTOR3 = 7
} LeanClrHostValueKind;

typedef struct LeanClrHostVector3
{
    float x;
    float y;
    float z;
} LeanClrHostVector3;

typedef struct LeanClrHostValue
{
    LeanClrHostValueKind kind;
    union
    {
        int32_t bool_value;
        int32_t int32_value;
        float float32_value;
        double float64_value;
        const char* string_value;
        LeanClrHostHandle handle_value;
        LeanClrHostVector3 vector3_value;
    } data;
} LeanClrHostValue;

typedef struct LeanClrHostBridgeError
{
    LeanClrHostBridgeStatus status;
    const char* message;
} LeanClrHostBridgeError;

typedef void (*LeanClrHostBridgeLogFn)(void* user_data, int32_t level, const char* message);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeInvokeManagedEntryFn)(void* user_data,
                                                                         const char* assembly_name,
                                                                         const char* type_name,
                                                                         const char* method_name,
                                                                         LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeCreateHandleFn)(void* user_data,
                                                                   const char* type_name,
                                                                   const char* debug_name,
                                                                   LeanClrHostHandle* out_handle,
                                                                   LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeRetainHandleFn)(void* user_data,
                                                                   LeanClrHostHandle handle,
                                                                   LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeReleaseHandleFn)(void* user_data,
                                                                    LeanClrHostHandle handle,
                                                                    LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeIsHandleAliveFn)(void* user_data,
                                                                    LeanClrHostHandle handle,
                                                                    int32_t* out_alive,
                                                                    LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeNotifyHandleDestroyedFn)(void* user_data,
                                                                           LeanClrHostHandle handle,
                                                                           LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeDispatchCallbackFn)(void* callback_data,
                                                                       LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgePostToMainThreadFn)(void* user_data,
                                                                       LeanClrHostBridgeDispatchCallbackFn callback,
                                                                       void* callback_data,
                                                                       uint64_t* out_ticket,
                                                                       LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgePumpMainThreadFn)(void* user_data,
                                                                     uint32_t max_items,
                                                                     uint32_t* out_executed,
                                                                     LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeCallMainThreadSyncFn)(void* user_data,
                                                                         LeanClrHostBridgeDispatchCallbackFn callback,
                                                                         void* callback_data,
                                                                         LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeSubscribeEventFn)(void* user_data,
                                                                     LeanClrHostHandle source_handle,
                                                                     const char* event_name,
                                                                     LeanClrHostBridgeDispatchCallbackFn callback,
                                                                     void* callback_data,
                                                                     LeanClrHostSubscription* out_subscription,
                                                                     LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeUnsubscribeEventFn)(void* user_data,
                                                                       LeanClrHostSubscription subscription,
                                                                       LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeTriggerEventFn)(void* user_data,
                                                                   LeanClrHostSubscription subscription,
                                                                   LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeGetPropertyFn)(void* user_data,
                                                                  LeanClrHostHandle handle,
                                                                  const char* property_name,
                                                                  LeanClrHostValue* out_value,
                                                                  LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeSetPropertyFn)(void* user_data,
                                                                  LeanClrHostHandle handle,
                                                                  const char* property_name,
                                                                  const LeanClrHostValue* value,
                                                                  LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeInvokeCommandFn)(void* user_data,
                                                                    LeanClrHostHandle handle,
                                                                    const char* command_name,
                                                                    const LeanClrHostValue* args,
                                                                    uint32_t arg_count,
                                                                    LeanClrHostValue* out_value,
                                                                    LeanClrHostBridgeError* error);

typedef LeanClrHostBridgeStatus (*LeanClrHostBridgeReportManagedExceptionFn)(void* user_data,
                                                                            const char* exception_type,
                                                                            const char* message,
                                                                            const char* stack_trace,
                                                                            LeanClrHostBridgeError* error);

typedef struct LeanClrHostBridgeFunctions
{
    uint32_t size;
    uint32_t abi_version;
    uint64_t capabilities;
    void* user_data;
    LeanClrHostBridgeLogFn log;
    LeanClrHostBridgeInvokeManagedEntryFn invoke_managed_entry;
    LeanClrHostBridgeCreateHandleFn create_handle;
    LeanClrHostBridgeRetainHandleFn retain_handle;
    LeanClrHostBridgeReleaseHandleFn release_handle;
    LeanClrHostBridgeIsHandleAliveFn is_handle_alive;
    LeanClrHostBridgeNotifyHandleDestroyedFn notify_handle_destroyed;
    LeanClrHostBridgePostToMainThreadFn post_to_main_thread;
    LeanClrHostBridgePumpMainThreadFn pump_main_thread;
    LeanClrHostBridgeCallMainThreadSyncFn call_main_thread_sync;
    LeanClrHostBridgeSubscribeEventFn subscribe_event;
    LeanClrHostBridgeUnsubscribeEventFn unsubscribe_event;
    LeanClrHostBridgeTriggerEventFn trigger_event;
    LeanClrHostBridgeGetPropertyFn get_property;
    LeanClrHostBridgeSetPropertyFn set_property;
    LeanClrHostBridgeInvokeCommandFn invoke_command;
    LeanClrHostBridgeReportManagedExceptionFn report_managed_exception;
} LeanClrHostBridgeFunctions;

static inline void LeanClrHostBridge_SetError(LeanClrHostBridgeError* error,
                                              LeanClrHostBridgeStatus status,
                                              const char* message)
{
    if (error == 0)
    {
        return;
    }

    error->status = status;
    error->message = message;
}

static inline LeanClrHostBridgeStatus LeanClrHostBridge_ValidateFunctions(const LeanClrHostBridgeFunctions* functions,
                                                                          uint64_t required_capabilities,
                                                                          LeanClrHostBridgeError* error)
{
    if (functions == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host bridge function table is null");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    if (functions->size < sizeof(LeanClrHostBridgeFunctions))
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host bridge function table is too small");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    if (functions->abi_version != LEANCLR_HOST_BRIDGE_ABI_VERSION)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_UNSUPPORTED_VERSION, "host bridge ABI version is not supported");
        return LEANCLR_HOST_BRIDGE_UNSUPPORTED_VERSION;
    }

    if ((functions->capabilities & required_capabilities) != required_capabilities)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_MISSING_CAPABILITY, "host bridge capability is missing");
        return LEANCLR_HOST_BRIDGE_MISSING_CAPABILITY;
    }

    if (((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_LOGGING) != 0 && functions->log == 0) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY) != 0 && functions->invoke_managed_entry == 0) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY) != 0 &&
         (functions->create_handle == 0 ||
          functions->retain_handle == 0 ||
          functions->release_handle == 0 ||
          functions->is_handle_alive == 0 ||
          functions->notify_handle_destroyed == 0)) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH) != 0 &&
         (functions->post_to_main_thread == 0 ||
          functions->pump_main_thread == 0 ||
          functions->call_main_thread_sync == 0)) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK) != 0 &&
         (functions->subscribe_event == 0 ||
          functions->unsubscribe_event == 0 ||
          functions->trigger_event == 0)) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_VALUE_MARSHAL) != 0 &&
         (functions->get_property == 0 ||
          functions->set_property == 0 ||
          functions->invoke_command == 0)) ||
        ((functions->capabilities & LEANCLR_HOST_BRIDGE_CAP_DIAGNOSTICS) != 0 &&
         functions->report_managed_exception == 0))
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host bridge function pointer is missing");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, 0);
    return LEANCLR_HOST_BRIDGE_OK;
}

#ifdef __cplusplus
}
#endif

#endif
