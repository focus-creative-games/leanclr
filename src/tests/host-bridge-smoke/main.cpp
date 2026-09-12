#include "host/leanclr_host_bridge.h"

#include <cstring>
#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
struct MockHandleRecord
{
    std::string type_name;
    std::string debug_name;
    uint32_t ref_count = 1;
    bool alive = true;
    bool active = true;
    int32_t health = 100;
    float speed = 1.0f;
    LeanClrHostVector3 position{0.0f, 0.0f, 0.0f};
};

struct MockDispatchTask
{
    uint64_t ticket = 0;
    LeanClrHostBridgeDispatchCallbackFn callback = nullptr;
    void* callback_data = nullptr;
};

struct MockEventSubscription
{
    LeanClrHostHandle source_handle = 0;
    std::string event_name;
    LeanClrHostBridgeDispatchCallbackFn callback = nullptr;
    void* callback_data = nullptr;
    bool active = true;
};

struct MockHostState
{
    int log_count = 0;
    int invoke_count = 0;
    LeanClrHostHandle next_handle = 100;
    uint64_t next_dispatch_ticket = 1;
    LeanClrHostSubscription next_subscription = 1;
    bool is_pumping_main_thread = false;
    std::string last_entry;
    std::unordered_map<LeanClrHostHandle, MockHandleRecord> handles;
    std::unordered_map<LeanClrHostSubscription, MockEventSubscription> subscriptions;
    std::deque<MockDispatchTask> dispatch_queue;
    std::vector<int> dispatch_order;
    std::vector<std::string> logs;
    std::string last_managed_exception;
};

struct MockDispatchPayload
{
    MockHostState* state = nullptr;
    int value = 0;
    bool fail = false;
};

struct MockEngineAdapter
{
    LeanClrHostBridgeFunctions* functions = nullptr;
    LeanClrHostHandle player_handle = 0;
    LeanClrHostSubscription ready_subscription = 0;
};

void mock_log(void* user_data, int32_t level, const char* message)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || message == nullptr)
    {
        return;
    }

    if (level >= 0)
    {
        ++state->log_count;
        state->logs.push_back(std::to_string(level) + ":" + message);
    }
}

LeanClrHostBridgeStatus mock_invoke_managed_entry(void* user_data,
                                                  const char* assembly_name,
                                                  const char* type_name,
                                                  const char* method_name,
                                                  LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || assembly_name == nullptr || type_name == nullptr || method_name == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "managed entry request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    ++state->invoke_count;
    state->last_entry = std::string(assembly_name) + "!" + type_name + "::" + method_name;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

MockHandleRecord* find_live_handle(MockHostState* state, LeanClrHostHandle handle, LeanClrHostBridgeError* error)
{
    if (state == nullptr || handle == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host handle request is incomplete");
        return nullptr;
    }

    auto it = state->handles.find(handle);
    if (it == state->handles.end() || !it->second.alive || it->second.ref_count == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED, "host handle is no longer alive");
        return nullptr;
    }

    return &it->second;
}

LeanClrHostBridgeStatus mock_create_handle(void* user_data,
                                           const char* type_name,
                                           const char* debug_name,
                                           LeanClrHostHandle* out_handle,
                                           LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || type_name == nullptr || out_handle == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host handle creation request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    const LeanClrHostHandle handle = state->next_handle++;
    MockHandleRecord record;
    record.type_name = type_name;
    record.debug_name = debug_name == nullptr ? "" : debug_name;
    state->handles.emplace(handle, record);
    *out_handle = handle;

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_retain_handle(void* user_data, LeanClrHostHandle handle, LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    auto* record = find_live_handle(state, handle, error);
    if (record == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }

    ++record->ref_count;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_release_handle(void* user_data, LeanClrHostHandle handle, LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || handle == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host handle release request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    auto it = state->handles.find(handle);
    if (it == state->handles.end())
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED, "host handle is unknown");
        return LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED;
    }

    if (it->second.ref_count > 0)
    {
        --it->second.ref_count;
    }
    if (it->second.ref_count == 0)
    {
        it->second.alive = false;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_is_handle_alive(void* user_data,
                                             LeanClrHostHandle handle,
                                             int32_t* out_alive,
                                             LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || out_alive == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host handle query request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    auto it = state->handles.find(handle);
    *out_alive = it != state->handles.end() && it->second.alive && it->second.ref_count > 0 ? 1 : 0;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_notify_handle_destroyed(void* user_data,
                                                     LeanClrHostHandle handle,
                                                     LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || handle == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host handle destroy notification is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    auto it = state->handles.find(handle);
    if (it == state->handles.end())
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED, "host handle is unknown");
        return LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED;
    }

    it->second.alive = false;
    it->second.ref_count = 0;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_dispatch_callback(void* callback_data, LeanClrHostBridgeError* error)
{
    auto* payload = static_cast<MockDispatchPayload*>(callback_data);
    if (payload == nullptr || payload->state == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "main-thread callback payload is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    payload->state->dispatch_order.push_back(payload->value);
    if (payload->fail)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_MANAGED_EXCEPTION, "managed continuation failed");
        return LEANCLR_HOST_BRIDGE_MANAGED_EXCEPTION;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_post_to_main_thread(void* user_data,
                                                 LeanClrHostBridgeDispatchCallbackFn callback,
                                                 void* callback_data,
                                                 uint64_t* out_ticket,
                                                 LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || callback == nullptr || out_ticket == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "main-thread post request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    MockDispatchTask task;
    task.ticket = state->next_dispatch_ticket++;
    task.callback = callback;
    task.callback_data = callback_data;
    state->dispatch_queue.push_back(task);
    *out_ticket = task.ticket;

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_pump_main_thread(void* user_data,
                                              uint32_t max_items,
                                              uint32_t* out_executed,
                                              LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || out_executed == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "main-thread pump request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    *out_executed = 0;
    state->is_pumping_main_thread = true;
    while (!state->dispatch_queue.empty() && (max_items == 0 || *out_executed < max_items))
    {
        const auto task = state->dispatch_queue.front();
        state->dispatch_queue.pop_front();
        ++(*out_executed);

        auto status = task.callback(task.callback_data, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            state->is_pumping_main_thread = false;
            return status;
        }
    }

    state->is_pumping_main_thread = false;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_call_main_thread_sync(void* user_data,
                                                   LeanClrHostBridgeDispatchCallbackFn callback,
                                                   void* callback_data,
                                                   LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || callback == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "main-thread sync request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }
    if (state->is_pumping_main_thread)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_REENTRANT_CALL, "synchronous main-thread call is not allowed while pumping");
        return LEANCLR_HOST_BRIDGE_REENTRANT_CALL;
    }

    return callback(callback_data, error);
}

LeanClrHostBridgeStatus mock_subscribe_event(void* user_data,
                                             LeanClrHostHandle source_handle,
                                             const char* event_name,
                                             LeanClrHostBridgeDispatchCallbackFn callback,
                                             void* callback_data,
                                             LeanClrHostSubscription* out_subscription,
                                             LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || event_name == nullptr || callback == nullptr || out_subscription == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "event subscription request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }
    if (find_live_handle(state, source_handle, error) == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }

    const LeanClrHostSubscription subscription = state->next_subscription++;
    MockEventSubscription record;
    record.source_handle = source_handle;
    record.event_name = event_name;
    record.callback = callback;
    record.callback_data = callback_data;
    state->subscriptions.emplace(subscription, record);
    *out_subscription = subscription;

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_unsubscribe_event(void* user_data,
                                               LeanClrHostSubscription subscription,
                                               LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || subscription == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "event unsubscribe request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    auto it = state->subscriptions.find(subscription);
    if (it != state->subscriptions.end())
    {
        it->second.active = false;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_trigger_event(void* user_data,
                                           LeanClrHostSubscription subscription,
                                           LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || subscription == 0)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "event trigger request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    auto it = state->subscriptions.find(subscription);
    if (it == state->subscriptions.end() || !it->second.active)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
        return LEANCLR_HOST_BRIDGE_OK;
    }
    if (find_live_handle(state, it->second.source_handle, error) == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }

    uint64_t ignored_ticket = 0;
    return mock_post_to_main_thread(state, it->second.callback, it->second.callback_data, &ignored_ticket, error);
}

LeanClrHostValue make_bool_value(bool value)
{
    LeanClrHostValue result{};
    result.kind = LEANCLR_HOST_VALUE_BOOL;
    result.data.bool_value = value ? 1 : 0;
    return result;
}

LeanClrHostValue make_int32_value(int32_t value)
{
    LeanClrHostValue result{};
    result.kind = LEANCLR_HOST_VALUE_INT32;
    result.data.int32_value = value;
    return result;
}

LeanClrHostValue make_float32_value(float value)
{
    LeanClrHostValue result{};
    result.kind = LEANCLR_HOST_VALUE_FLOAT32;
    result.data.float32_value = value;
    return result;
}

LeanClrHostValue make_string_value(const char* value)
{
    LeanClrHostValue result{};
    result.kind = LEANCLR_HOST_VALUE_STRING;
    result.data.string_value = value;
    return result;
}

LeanClrHostValue make_vector3_value(float x, float y, float z)
{
    LeanClrHostValue result{};
    result.kind = LEANCLR_HOST_VALUE_VECTOR3;
    result.data.vector3_value = LeanClrHostVector3{x, y, z};
    return result;
}

LeanClrHostBridgeStatus require_value_kind(const LeanClrHostValue* value,
                                           LeanClrHostValueKind kind,
                                           LeanClrHostBridgeError* error)
{
    if (value == nullptr || value->kind != kind)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host value kind does not match property contract");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_get_property(void* user_data,
                                          LeanClrHostHandle handle,
                                          const char* property_name,
                                          LeanClrHostValue* out_value,
                                          LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    auto* record = find_live_handle(state, handle, error);
    if (record == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }
    if (property_name == nullptr || out_value == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host property get request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    if (std::strcmp(property_name, "active") == 0)
    {
        *out_value = make_bool_value(record->active);
    }
    else if (std::strcmp(property_name, "health") == 0)
    {
        *out_value = make_int32_value(record->health);
    }
    else if (std::strcmp(property_name, "speed") == 0)
    {
        *out_value = make_float32_value(record->speed);
    }
    else if (std::strcmp(property_name, "title") == 0)
    {
        *out_value = make_string_value(record->debug_name.c_str());
    }
    else if (std::strcmp(property_name, "position") == 0)
    {
        *out_value = make_vector3_value(record->position.x, record->position.y, record->position.z);
    }
    else
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host property is not supported");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_set_property(void* user_data,
                                          LeanClrHostHandle handle,
                                          const char* property_name,
                                          const LeanClrHostValue* value,
                                          LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    auto* record = find_live_handle(state, handle, error);
    if (record == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }
    if (property_name == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host property set request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    if (std::strcmp(property_name, "active") == 0)
    {
        auto status = require_value_kind(value, LEANCLR_HOST_VALUE_BOOL, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            return status;
        }
        record->active = value->data.bool_value != 0;
    }
    else if (std::strcmp(property_name, "health") == 0)
    {
        auto status = require_value_kind(value, LEANCLR_HOST_VALUE_INT32, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            return status;
        }
        record->health = value->data.int32_value;
    }
    else if (std::strcmp(property_name, "speed") == 0)
    {
        auto status = require_value_kind(value, LEANCLR_HOST_VALUE_FLOAT32, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            return status;
        }
        record->speed = value->data.float32_value;
    }
    else if (std::strcmp(property_name, "title") == 0)
    {
        auto status = require_value_kind(value, LEANCLR_HOST_VALUE_STRING, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            return status;
        }
        record->debug_name = value->data.string_value == nullptr ? "" : value->data.string_value;
    }
    else if (std::strcmp(property_name, "position") == 0)
    {
        auto status = require_value_kind(value, LEANCLR_HOST_VALUE_VECTOR3, error);
        if (status != LEANCLR_HOST_BRIDGE_OK)
        {
            return status;
        }
        record->position = value->data.vector3_value;
    }
    else
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host property is not supported");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_invoke_command(void* user_data,
                                            LeanClrHostHandle handle,
                                            const char* command_name,
                                            const LeanClrHostValue* args,
                                            uint32_t arg_count,
                                            LeanClrHostValue* out_value,
                                            LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    auto* record = find_live_handle(state, handle, error);
    if (record == nullptr)
    {
        return error == nullptr ? LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED : error->status;
    }
    if (command_name == nullptr || out_value == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host command request is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    if (std::strcmp(command_name, "MoveBy") == 0)
    {
        if (arg_count != 1)
        {
            LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "MoveBy expects one argument");
            return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
        }
        if (require_value_kind(args, LEANCLR_HOST_VALUE_VECTOR3, error) != LEANCLR_HOST_BRIDGE_OK)
        {
            return error == nullptr ? LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT : error->status;
        }
        record->position.x += args[0].data.vector3_value.x;
        record->position.y += args[0].data.vector3_value.y;
        record->position.z += args[0].data.vector3_value.z;
        *out_value = make_vector3_value(record->position.x, record->position.y, record->position.z);
    }
    else if (std::strcmp(command_name, "Damage") == 0)
    {
        if (arg_count != 1)
        {
            LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "Damage expects one argument");
            return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
        }
        if (require_value_kind(args, LEANCLR_HOST_VALUE_INT32, error) != LEANCLR_HOST_BRIDGE_OK)
        {
            return error == nullptr ? LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT : error->status;
        }
        record->health -= args[0].data.int32_value;
        *out_value = make_int32_value(record->health);
    }
    else if (std::strcmp(command_name, "Describe") == 0)
    {
        if (arg_count != 0)
        {
            LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "Describe does not accept arguments");
            return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
        }
        *out_value = make_string_value(record->debug_name.c_str());
    }
    else
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "host command is not supported");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

LeanClrHostBridgeStatus mock_report_managed_exception(void* user_data,
                                                      const char* exception_type,
                                                      const char* message,
                                                      const char* stack_trace,
                                                      LeanClrHostBridgeError* error)
{
    auto* state = static_cast<MockHostState*>(user_data);
    if (state == nullptr || exception_type == nullptr || message == nullptr || stack_trace == nullptr)
    {
        LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT, "managed exception diagnostic is incomplete");
        return LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT;
    }

    state->last_managed_exception = std::string(exception_type) + ":" + message + ":" + stack_trace;
    LeanClrHostBridge_SetError(error, LEANCLR_HOST_BRIDGE_OK, nullptr);
    return LEANCLR_HOST_BRIDGE_OK;
}

bool require(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "host-bridge-smoke: " << message << std::endl;
        return false;
    }

    return true;
}

LeanClrHostBridgeFunctions make_mock_functions(MockHostState* state)
{
    LeanClrHostBridgeFunctions functions{};
    functions.size = sizeof(functions);
    functions.abi_version = LEANCLR_HOST_BRIDGE_ABI_VERSION;
    functions.capabilities = LEANCLR_HOST_BRIDGE_CAP_LOGGING |
                             LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY |
                             LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY |
                             LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH |
                             LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK |
                             LEANCLR_HOST_BRIDGE_CAP_VALUE_MARSHAL |
                             LEANCLR_HOST_BRIDGE_CAP_DIAGNOSTICS;
    functions.user_data = state;
    functions.log = mock_log;
    functions.invoke_managed_entry = mock_invoke_managed_entry;
    functions.create_handle = mock_create_handle;
    functions.retain_handle = mock_retain_handle;
    functions.release_handle = mock_release_handle;
    functions.is_handle_alive = mock_is_handle_alive;
    functions.notify_handle_destroyed = mock_notify_handle_destroyed;
    functions.post_to_main_thread = mock_post_to_main_thread;
    functions.pump_main_thread = mock_pump_main_thread;
    functions.call_main_thread_sync = mock_call_main_thread_sync;
    functions.subscribe_event = mock_subscribe_event;
    functions.unsubscribe_event = mock_unsubscribe_event;
    functions.trigger_event = mock_trigger_event;
    functions.get_property = mock_get_property;
    functions.set_property = mock_set_property;
    functions.invoke_command = mock_invoke_command;
    functions.report_managed_exception = mock_report_managed_exception;
    return functions;
}

bool run_abi_skeleton()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(
        &functions,
        LEANCLR_HOST_BRIDGE_CAP_LOGGING | LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY,
        &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && error.status == LEANCLR_HOST_BRIDGE_OK,
                 "valid function table should be accepted"))
    {
        return false;
    }

    functions.log(functions.user_data, 1, "host bridge mock initialized");
    if (!require(state.log_count == 1, "logging callback was not invoked"))
    {
        return false;
    }

    status = functions.invoke_managed_entry(functions.user_data,
                                           "ManagedNet10.HostBridgeSmoke",
                                           "ManagedNet10.HostBridgeSmoke.Program",
                                           "Run",
                                           &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.invoke_count == 1,
                 "managed entry callback was not invoked"))
    {
        return false;
    }
    if (!require(state.last_entry == "ManagedNet10.HostBridgeSmoke!ManagedNet10.HostBridgeSmoke.Program::Run",
                 "managed entry identity was not preserved"))
    {
        return false;
    }

    auto invalid_version = functions;
    invalid_version.abi_version = LEANCLR_HOST_BRIDGE_ABI_VERSION + 1u;
    status = LeanClrHostBridge_ValidateFunctions(&invalid_version, LEANCLR_HOST_BRIDGE_CAP_LOGGING, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_UNSUPPORTED_VERSION && error.message != nullptr,
                 "unsupported ABI version should return a diagnostic"))
    {
        return false;
    }

    auto missing_capability = functions;
    missing_capability.capabilities = LEANCLR_HOST_BRIDGE_CAP_LOGGING;
    status = LeanClrHostBridge_ValidateFunctions(
        &missing_capability,
        LEANCLR_HOST_BRIDGE_CAP_LOGGING | LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY,
        &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_MISSING_CAPABILITY && error.message != nullptr,
                 "missing capability should return a diagnostic"))
    {
        return false;
    }

    auto missing_callback = functions;
    missing_callback.invoke_managed_entry = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_callback, LEANCLR_HOST_BRIDGE_CAP_INVOKE_MANAGED_ENTRY, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing callback should return a diagnostic"))
    {
        return false;
    }

    return true;
}

bool run_handle_registry()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(&functions, LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "handle registry function table should be accepted"))
    {
        return false;
    }

    LeanClrHostHandle handle = 0;
    status = functions.create_handle(functions.user_data, "Mock.Node", "Player", &handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && handle != 0, "handle creation should return an opaque handle"))
    {
        return false;
    }
    if (!require(state.handles[handle].type_name == "Mock.Node" && state.handles[handle].debug_name == "Player",
                 "handle metadata should stay on the host side"))
    {
        return false;
    }

    int32_t alive = 0;
    status = functions.is_handle_alive(functions.user_data, handle, &alive, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && alive == 1, "created handle should be alive"))
    {
        return false;
    }

    status = functions.retain_handle(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.handles[handle].ref_count == 2,
                 "retain should increment the host-side ref count"))
    {
        return false;
    }

    status = functions.release_handle(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.handles[handle].ref_count == 1,
                 "release should decrement the host-side ref count"))
    {
        return false;
    }

    status = functions.notify_handle_destroyed(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "host destroy notification should invalidate the handle"))
    {
        return false;
    }

    status = functions.is_handle_alive(functions.user_data, handle, &alive, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && alive == 0, "destroyed handle should not be alive"))
    {
        return false;
    }

    status = functions.retain_handle(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED && error.message != nullptr,
                 "retain after host destroy should return an ObjectDisposed-style diagnostic"))
    {
        return false;
    }

    status = functions.release_handle(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "release after host destroy should be idempotent"))
    {
        return false;
    }
    status = functions.release_handle(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "duplicate release should stay idempotent"))
    {
        return false;
    }

    LeanClrHostHandle released_handle = 0;
    status = functions.create_handle(functions.user_data, "Mock.Resource", "Temp", &released_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && released_handle != 0,
                 "second handle creation should succeed"))
    {
        return false;
    }
    status = functions.release_handle(functions.user_data, released_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "final release should succeed"))
    {
        return false;
    }
    status = functions.release_handle(functions.user_data, released_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "repeat final release should be idempotent"))
    {
        return false;
    }

    auto missing_registry = functions;
    missing_registry.create_handle = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_registry, LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing handle registry callback should return a diagnostic"))
    {
        return false;
    }

    return true;
}

bool run_dispatcher()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(&functions, LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "dispatcher function table should be accepted"))
    {
        return false;
    }

    MockDispatchPayload first{&state, 1, false};
    MockDispatchPayload second{&state, 2, false};
    uint64_t first_ticket = 0;
    uint64_t second_ticket = 0;
    status = functions.post_to_main_thread(functions.user_data, mock_dispatch_callback, &first, &first_ticket, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && first_ticket == 1, "first main-thread post should return a ticket"))
    {
        return false;
    }
    status = functions.post_to_main_thread(functions.user_data, mock_dispatch_callback, &second, &second_ticket, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && second_ticket == 2, "second main-thread post should return the next ticket"))
    {
        return false;
    }

    uint32_t executed = 0;
    status = functions.pump_main_thread(functions.user_data, 1, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && executed == 1 && state.dispatch_order.size() == 1 && state.dispatch_order[0] == 1,
                 "single-item pump should execute only the first queued continuation"))
    {
        return false;
    }

    status = functions.pump_main_thread(functions.user_data, 0, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && executed == 1 && state.dispatch_order.size() == 2 && state.dispatch_order[1] == 2,
                 "next pump should preserve FIFO ordering"))
    {
        return false;
    }

    MockDispatchPayload sync_payload{&state, 3, false};
    status = functions.call_main_thread_sync(functions.user_data, mock_dispatch_callback, &sync_payload, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.dispatch_order.size() == 3 && state.dispatch_order[2] == 3,
                 "synchronous main-thread call should return callback results"))
    {
        return false;
    }

    state.is_pumping_main_thread = true;
    status = functions.call_main_thread_sync(functions.user_data, mock_dispatch_callback, &sync_payload, &error);
    state.is_pumping_main_thread = false;
    if (!require(status == LEANCLR_HOST_BRIDGE_REENTRANT_CALL && error.message != nullptr,
                 "synchronous reentry while pumping should be rejected"))
    {
        return false;
    }

    MockDispatchPayload failing{&state, 4, true};
    uint64_t failing_ticket = 0;
    status = functions.post_to_main_thread(functions.user_data, mock_dispatch_callback, &failing, &failing_ticket, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && failing_ticket == 3, "failing continuation should still be queued"))
    {
        return false;
    }
    status = functions.pump_main_thread(functions.user_data, 1, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_MANAGED_EXCEPTION && executed == 1 && error.message != nullptr,
                 "managed exception should be returned as dispatcher diagnostic"))
    {
        return false;
    }

    auto missing_dispatcher = functions;
    missing_dispatcher.post_to_main_thread = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_dispatcher, LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing dispatcher callback should return a diagnostic"))
    {
        return false;
    }

    return true;
}

bool run_event_callback()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(&functions, LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "event callback function table should be accepted"))
    {
        return false;
    }

    LeanClrHostHandle source_handle = 0;
    status = functions.create_handle(functions.user_data, "Mock.Button", "Start", &source_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && source_handle != 0, "event source handle should be created"))
    {
        return false;
    }

    MockDispatchPayload click_payload{&state, 10, false};
    LeanClrHostSubscription click_subscription = 0;
    status = functions.subscribe_event(functions.user_data,
                                       source_handle,
                                       "OnClick",
                                       mock_dispatch_callback,
                                       &click_payload,
                                       &click_subscription,
                                       &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && click_subscription != 0,
                 "event subscription should return a token"))
    {
        return false;
    }

    status = functions.trigger_event(functions.user_data, click_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.dispatch_order.empty(),
                 "event trigger should enqueue the callback instead of invoking it inline"))
    {
        return false;
    }

    uint32_t executed = 0;
    status = functions.pump_main_thread(functions.user_data, 0, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && executed == 1 && state.dispatch_order.size() == 1 && state.dispatch_order[0] == 10,
                 "event callback should run through the main-thread dispatcher"))
    {
        return false;
    }

    status = functions.unsubscribe_event(functions.user_data, click_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "event unsubscribe should succeed"))
    {
        return false;
    }
    status = functions.unsubscribe_event(functions.user_data, click_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "duplicate event unsubscribe should be idempotent"))
    {
        return false;
    }
    status = functions.trigger_event(functions.user_data, click_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "triggering an inactive subscription should be a no-op"))
    {
        return false;
    }
    status = functions.pump_main_thread(functions.user_data, 0, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && executed == 0 && state.dispatch_order.size() == 1,
                 "unsubscribed event should not enqueue callbacks"))
    {
        return false;
    }

    MockDispatchPayload failing_payload{&state, 20, true};
    LeanClrHostSubscription failing_subscription = 0;
    status = functions.subscribe_event(functions.user_data,
                                       source_handle,
                                       "OnFail",
                                       mock_dispatch_callback,
                                       &failing_payload,
                                       &failing_subscription,
                                       &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && failing_subscription != 0,
                 "failing event subscription should return a token"))
    {
        return false;
    }
    status = functions.trigger_event(functions.user_data, failing_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "failing event should still enqueue"))
    {
        return false;
    }
    status = functions.pump_main_thread(functions.user_data, 1, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_MANAGED_EXCEPTION && executed == 1 && error.message != nullptr,
                 "event callback managed exception should be returned by dispatcher pump"))
    {
        return false;
    }

    status = functions.notify_handle_destroyed(functions.user_data, source_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "destroying an event source should succeed"))
    {
        return false;
    }
    status = functions.trigger_event(functions.user_data, failing_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED && error.message != nullptr,
                 "triggering an event on a destroyed source should return a diagnostic"))
    {
        return false;
    }

    auto missing_event = functions;
    missing_event.subscribe_event = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_event, LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing event callback should return a diagnostic"))
    {
        return false;
    }

    return true;
}

bool run_engine_adapter()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    MockEngineAdapter adapter{&functions};
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(
        adapter.functions,
        LEANCLR_HOST_BRIDGE_CAP_HANDLE_REGISTRY |
            LEANCLR_HOST_BRIDGE_CAP_MAIN_THREAD_DISPATCH |
            LEANCLR_HOST_BRIDGE_CAP_EVENT_CALLBACK,
        &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "engine adapter function table should be accepted"))
    {
        return false;
    }

    status = adapter.functions->create_handle(adapter.functions->user_data,
                                              "MockEngine.Node",
                                              "Player",
                                              &adapter.player_handle,
                                              &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && adapter.player_handle != 0,
                 "engine adapter should create an opaque node handle"))
    {
        return false;
    }
    if (!require(state.handles[adapter.player_handle].type_name == "MockEngine.Node",
                 "engine adapter should keep native object identity behind the handle"))
    {
        return false;
    }

    MockDispatchPayload ready_payload{&state, 42, false};
    status = adapter.functions->subscribe_event(adapter.functions->user_data,
                                                adapter.player_handle,
                                                "Ready",
                                                mock_dispatch_callback,
                                                &ready_payload,
                                                &adapter.ready_subscription,
                                                &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && adapter.ready_subscription != 0,
                 "engine adapter should subscribe by token"))
    {
        return false;
    }

    status = adapter.functions->trigger_event(adapter.functions->user_data, adapter.ready_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && state.dispatch_order.empty(),
                 "engine event should enqueue through the host dispatcher"))
    {
        return false;
    }

    uint32_t executed = 0;
    status = adapter.functions->pump_main_thread(adapter.functions->user_data, 0, &executed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && executed == 1 && state.dispatch_order.size() == 1 && state.dispatch_order[0] == 42,
                 "engine event should run on the main-thread pump"))
    {
        return false;
    }

    status = adapter.functions->notify_handle_destroyed(adapter.functions->user_data, adapter.player_handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "engine destroy should invalidate the handle"))
    {
        return false;
    }

    int32_t alive = 1;
    status = adapter.functions->is_handle_alive(adapter.functions->user_data, adapter.player_handle, &alive, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && alive == 0,
                 "engine adapter should expose destroyed objects as not alive"))
    {
        return false;
    }

    status = adapter.functions->trigger_event(adapter.functions->user_data, adapter.ready_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED && error.message != nullptr,
                 "engine event on a destroyed source should return a diagnostic"))
    {
        return false;
    }

    status = adapter.functions->unsubscribe_event(adapter.functions->user_data, adapter.ready_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "engine adapter unsubscribe should succeed"))
    {
        return false;
    }
    status = adapter.functions->unsubscribe_event(adapter.functions->user_data, adapter.ready_subscription, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "engine adapter duplicate unsubscribe should be idempotent"))
    {
        return false;
    }

    return true;
}

bool run_value_marshal()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(&functions, LEANCLR_HOST_BRIDGE_CAP_VALUE_MARSHAL, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "value marshal function table should be accepted"))
    {
        return false;
    }

    LeanClrHostHandle handle = 0;
    status = functions.create_handle(functions.user_data, "MockEngine.Node", "Player", &handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK && handle != 0, "value marshal node handle should be created"))
    {
        return false;
    }

    LeanClrHostValue value{};
    status = functions.get_property(functions.user_data, handle, "title", &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_STRING &&
                     std::strcmp(value.data.string_value, "Player") == 0,
                 "string property get failed"))
    {
        return false;
    }

    auto health = make_int32_value(75);
    status = functions.set_property(functions.user_data, handle, "health", &health, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "int property set failed"))
    {
        return false;
    }
    status = functions.get_property(functions.user_data, handle, "health", &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_INT32 &&
                     value.data.int32_value == 75,
                 "int property get failed"))
    {
        return false;
    }

    auto active = make_bool_value(false);
    status = functions.set_property(functions.user_data, handle, "active", &active, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "bool property set failed"))
    {
        return false;
    }
    status = functions.get_property(functions.user_data, handle, "active", &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_BOOL &&
                     value.data.bool_value == 0,
                 "bool property get failed"))
    {
        return false;
    }

    auto speed = make_float32_value(3.5f);
    status = functions.set_property(functions.user_data, handle, "speed", &speed, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "float property set failed"))
    {
        return false;
    }
    status = functions.get_property(functions.user_data, handle, "speed", &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_FLOAT32 &&
                     value.data.float32_value == 3.5f,
                 "float property get failed"))
    {
        return false;
    }

    auto position = make_vector3_value(1.0f, 2.0f, 3.0f);
    status = functions.set_property(functions.user_data, handle, "position", &position, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "vector property set failed"))
    {
        return false;
    }

    auto delta = make_vector3_value(2.0f, 0.5f, -1.0f);
    status = functions.invoke_command(functions.user_data, handle, "MoveBy", &delta, 1, &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_VECTOR3 &&
                     value.data.vector3_value.x == 3.0f &&
                     value.data.vector3_value.y == 2.5f &&
                     value.data.vector3_value.z == 2.0f,
                 "vector command marshal failed"))
    {
        return false;
    }

    auto damage = make_int32_value(5);
    status = functions.invoke_command(functions.user_data, handle, "Damage", &damage, 1, &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     value.kind == LEANCLR_HOST_VALUE_INT32 &&
                     value.data.int32_value == 70,
                 "int command marshal failed"))
    {
        return false;
    }

    auto bad_value = make_string_value("wrong");
    status = functions.set_property(functions.user_data, handle, "health", &bad_value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "wrong value kind should return a diagnostic"))
    {
        return false;
    }

    auto missing_value_marshal = functions;
    missing_value_marshal.get_property = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_value_marshal, LEANCLR_HOST_BRIDGE_CAP_VALUE_MARSHAL, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing value marshal callback should return a diagnostic"))
    {
        return false;
    }

    status = functions.notify_handle_destroyed(functions.user_data, handle, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "destroying value marshal source should succeed"))
    {
        return false;
    }
    status = functions.get_property(functions.user_data, handle, "health", &value, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OBJECT_DISPOSED && error.message != nullptr,
                 "property get after destroy should return ObjectDisposed diagnostic"))
    {
        return false;
    }

    return true;
}

bool run_diagnostics()
{
    MockHostState state;
    auto functions = make_mock_functions(&state);
    LeanClrHostBridgeError error{};

    auto status = LeanClrHostBridge_ValidateFunctions(
        &functions,
        LEANCLR_HOST_BRIDGE_CAP_LOGGING | LEANCLR_HOST_BRIDGE_CAP_DIAGNOSTICS,
        &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK, "diagnostics function table should be accepted"))
    {
        return false;
    }

    functions.log(functions.user_data, LEANCLR_HOST_BRIDGE_LOG_INFO, "engine adapter started");
    functions.log(functions.user_data, LEANCLR_HOST_BRIDGE_LOG_WARNING, "engine adapter warning");
    functions.log(functions.user_data, LEANCLR_HOST_BRIDGE_LOG_ERROR, "engine adapter error");
    if (!require(state.logs.size() == 3 &&
                     state.logs[0] == "1:engine adapter started" &&
                     state.logs[1] == "2:engine adapter warning" &&
                     state.logs[2] == "3:engine adapter error",
                 "diagnostic log levels should be preserved"))
    {
        return false;
    }

    status = functions.report_managed_exception(functions.user_data,
                                                "System.InvalidOperationException",
                                                "managed callback failed",
                                                "ManagedNet10.Smoke.EngineNode.OnReady",
                                                &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_OK &&
                     state.last_managed_exception == "System.InvalidOperationException:managed callback failed:ManagedNet10.Smoke.EngineNode.OnReady",
                 "managed exception diagnostic should be reported"))
    {
        return false;
    }

    status = functions.report_managed_exception(functions.user_data,
                                                "System.InvalidOperationException",
                                                nullptr,
                                                "ManagedNet10.Smoke.EngineNode.OnReady",
                                                &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "incomplete managed exception diagnostic should fail clearly"))
    {
        return false;
    }

    auto missing_diagnostics = functions;
    missing_diagnostics.report_managed_exception = nullptr;
    status = LeanClrHostBridge_ValidateFunctions(&missing_diagnostics, LEANCLR_HOST_BRIDGE_CAP_DIAGNOSTICS, &error);
    if (!require(status == LEANCLR_HOST_BRIDGE_INVALID_ARGUMENT && error.message != nullptr,
                 "missing diagnostics callback should return a diagnostic"))
    {
        return false;
    }

    return true;
}
} // namespace

int main(int argc, char** argv)
{
    std::string scenario = "AbiSkeleton";
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--scenario") == 0 && i + 1 < argc)
        {
            scenario = argv[++i];
        }
    }

    if (scenario == "AbiSkeleton")
    {
        if (!run_abi_skeleton())
        {
            return 1;
        }

        std::cout << "ok! host bridge AbiSkeleton" << std::endl;
        return 0;
    }

    if (scenario == "HandleRegistry")
    {
        if (!run_handle_registry())
        {
            return 1;
        }

        std::cout << "ok! host bridge HandleRegistry" << std::endl;
        return 0;
    }

    if (scenario == "Dispatcher")
    {
        if (!run_dispatcher())
        {
            return 1;
        }

        std::cout << "ok! host bridge Dispatcher" << std::endl;
        return 0;
    }

    if (scenario == "EventCallback")
    {
        if (!run_event_callback())
        {
            return 1;
        }

        std::cout << "ok! host bridge EventCallback" << std::endl;
        return 0;
    }

    if (scenario == "EngineAdapter")
    {
        if (!run_engine_adapter())
        {
            return 1;
        }

        std::cout << "ok! host bridge EngineAdapter" << std::endl;
        return 0;
    }

    if (scenario == "ValueMarshal")
    {
        if (!run_value_marshal())
        {
            return 1;
        }

        std::cout << "ok! host bridge ValueMarshal" << std::endl;
        return 0;
    }

    if (scenario == "Diagnostics")
    {
        if (!run_diagnostics())
        {
            return 1;
        }

        std::cout << "ok! host bridge Diagnostics" << std::endl;
        return 0;
    }

    std::cerr << "host-bridge-smoke: unsupported scenario: " << scenario << std::endl;
    return 2;
}
