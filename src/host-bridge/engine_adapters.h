#pragma once
#include "public/engine_host.h"

namespace leanclr::host
{
// SDK-neutral lifecycle hooks. The engine integration owns the config/context,
// reports returned errors through its logger and calls exit explicitly.
// One persistent Godot autoload / Unreal module per process, never per scene.
class EngineOwner
{
  protected:
    LeanclrEngineSession* session_ = nullptr;

    LeanclrEngineStatus start(const LeanclrEngineConfig& config, LeanclrEngineError* error)
    {
        LeanclrEngineSession* value = nullptr;
        auto result = leanclr_engine_start(&config, &value, error);
        if (value)
            session_ = value;
        return result;
    }

    LeanclrEngineStatus tick(float delta, LeanclrEngineError* error)
    {
        return leanclr_engine_tick(session_, delta, error);
    }

    LeanclrEngineStatus stop(LeanclrEngineError* error)
    {
        return leanclr_engine_stop(session_, error);
    }

  public:
    EngineOwner() = default;
    EngineOwner(const EngineOwner&) = delete;
    EngineOwner& operator=(const EngineOwner&) = delete;

    LeanclrEngineState state() const
    {
        return leanclr_engine_state(session_);
    }
};

class GodotHostAdapter final : public EngineOwner
{
  public:
    LeanclrEngineStatus enter_tree(const LeanclrEngineConfig& config, LeanclrEngineError* error)
    {
        return start(config, error);
    }

    LeanclrEngineStatus process(float delta, LeanclrEngineError* error)
    {
        return tick(delta, error);
    }

    LeanclrEngineStatus exit_tree(LeanclrEngineError* error)
    {
        return stop(error);
    }
};

class UnrealHostAdapter final : public EngineOwner
{
  public:
    LeanclrEngineStatus startup_module(const LeanclrEngineConfig& config, LeanclrEngineError* error)
    {
        return start(config, error);
    }

    LeanclrEngineStatus tick_module(float delta, LeanclrEngineError* error)
    {
        return tick(delta, error);
    }

    LeanclrEngineStatus shutdown_module(LeanclrEngineError* error)
    {
        return stop(error);
    }
};
} // namespace leanclr::host
