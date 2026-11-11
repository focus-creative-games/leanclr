using System.Diagnostics.CodeAnalysis;

namespace ManagedNet10.Smoke;

public static class HostBridgeWrapperSmoke
{
    public static void Run()
    {
        var bridge = new MockHostBridge();
        var hostObject = HostObject.Create(bridge, "Mock.Node", "Player");
        Require(hostObject.Handle != 0, "host object wrapper did not receive a handle");
        Require(hostObject.IsAlive, "host object wrapper did not query liveness");

        var callbackCount = 0;
        using (var subscription = hostObject.Subscribe("OnClick", () => callbackCount++))
        {
            bridge.TriggerEvent(subscription.Token);
            Require(callbackCount == 0, "host event callback ran inline");
            HostDispatcher.PumpMainThread(bridge);
            Require(callbackCount == 1, "host event callback did not run through dispatcher");
        }

        bridge.TriggerLastEvent();
        HostDispatcher.PumpMainThread(bridge);
        Require(callbackCount == 1, "disposed event subscription should not enqueue callbacks");

        hostObject.Dispose();
        hostObject.Dispose();
        ExpectObjectDisposed(static state => ((HostObject)state!).EnsureAlive(), hostObject);

        var destroyedObject = HostObject.Create(bridge, "Mock.Node", "Destroyed");
        bridge.NotifyDestroyed(destroyedObject.Handle);
        ExpectObjectDisposed(static state => ((HostObject)state!).EnsureAlive(), destroyedObject);

        var failingObject = HostObject.Create(bridge, "Mock.Node", "FailingEvent");
        using (var failingSubscription = failingObject.Subscribe("OnFail", static () => throw new InvalidOperationException("managed callback failed")))
        {
            bridge.TriggerEvent(failingSubscription.Token);
            ExpectHostBridgeException(static state => HostDispatcher.PumpMainThread((MockHostBridge)state!),
                bridge,
                HostBridgeStatus.ManagedException);
        }

        bridge.FailNextCreate("host create failed");
        ExpectHostBridgeException(static state => HostObject.Create((MockHostBridge)state!, "Mock.Node", "Failure"),
            bridge,
            HostBridgeStatus.HostFailure);
    }

    private static void ExpectObjectDisposed(Action<object?> action, object? state)
    {
        try
        {
            action(state);
        }
        catch (ObjectDisposedException)
        {
            return;
        }

        throw new InvalidOperationException("expected ObjectDisposedException");
    }

    private static void ExpectHostBridgeException(Action<object?> action, object? state, HostBridgeStatus expectedStatus)
    {
        try
        {
            action(state);
        }
        catch (Exception ex)
        {
            if (ex is HostBridgeException hostBridgeException && hostBridgeException.Status == expectedStatus)
            {
                return;
            }
        }

        throw new InvalidOperationException("expected HostBridgeException");
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}

public static class EngineBindingSmoke
{
    public static void Run()
    {
        var bridge = new MockHostBridge();
        using var node = EngineNode.Create(bridge, "Player");
        Require(node.Handle != 0, "engine node did not receive a host handle");
        Require(node.Name == "Player", "engine node name was not preserved");

        var readyCount = 0;
        using (var ready = node.OnReady(() => readyCount++))
        {
            bridge.TriggerEvent(ready.Token);
            Require(readyCount == 0, "engine binding event ran inline");
            HostDispatcher.PumpMainThread(bridge);
            Require(readyCount == 1, "engine binding event did not use dispatcher");

            ready.Dispose();
            bridge.TriggerEvent(ready.Token);
            HostDispatcher.PumpMainThread(bridge);
            Require(readyCount == 1, "disposed engine binding event should not run");
        }

        bridge.NotifyDestroyed(node.Handle);
        ExpectObjectDisposed(static state => ((EngineNode)state!).EnsureAlive(), node);
        node.Dispose();
        node.Dispose();
    }

    private static void ExpectObjectDisposed(Action<object?> action, object? state)
    {
        try
        {
            action(state);
        }
        catch (ObjectDisposedException)
        {
            return;
        }

        throw new InvalidOperationException("expected ObjectDisposedException");
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}

public static class ValueMarshalSmoke
{
    public static void Run()
    {
        var bridge = new MockHostBridge();
        using var node = EngineNode.Create(bridge, "Player");

        Require(node.Active, "bool property initial value failed");
        node.Active = false;
        Require(!node.Active, "bool property round trip failed");

        node.Health = 75;
        Require(node.Health == 75, "int property round trip failed");

        node.Speed = 3.5f;
        Require(node.Speed == 3.5f, "float property round trip failed");

        node.Position = new HostVector3(1.0f, 2.0f, 3.0f);
        var moved = node.MoveBy(new HostVector3(2.0f, 0.5f, -1.0f));
        Require(moved.X == 3.0f && moved.Y == 2.5f && moved.Z == 2.0f, "vector command round trip failed");
        Require(node.Position.X == 3.0f && node.Position.Y == 2.5f && node.Position.Z == 2.0f, "vector property after command failed");

        Require(node.Damage(5) == 70, "int command return failed");
        Require(node.Health == 70, "int property after command failed");

        bridge.FailNextSetProperty(nameof(EngineNode.Health), "wrong value kind");
        ExpectHostBridgeException(static state => ((EngineNode)state!).Health = 1, node, HostBridgeStatus.InvalidArgument);

        bridge.NotifyDestroyed(node.Handle);
        ExpectObjectDisposed(static state => _ = ((EngineNode)state!).Health, node);
    }

    private static void ExpectObjectDisposed(Action<object?> action, object? state)
    {
        try
        {
            action(state);
        }
        catch (ObjectDisposedException)
        {
            return;
        }

        throw new InvalidOperationException("expected ObjectDisposedException");
    }

    private static void ExpectHostBridgeException(Action<object?> action, object? state, HostBridgeStatus expectedStatus)
    {
        try
        {
            action(state);
        }
        catch (Exception ex)
        {
            if (ex is HostBridgeException hostBridgeException && hostBridgeException.Status == expectedStatus)
            {
                return;
            }
        }

        throw new InvalidOperationException("expected HostBridgeException");
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}

public static class DiagnosticsSmoke
{
    public static void Run()
    {
        var bridge = new MockHostBridge();

        bridge.Log(HostLogLevel.Info, "engine adapter started");
        bridge.Log(HostLogLevel.Warning, "engine adapter warning");
        bridge.Log(HostLogLevel.Error, "engine adapter error");
        Require(bridge.Logs.Count == 3, "diagnostic log count failed");
        Require(bridge.Logs[0] == "Info:engine adapter started", "info log failed");
        Require(bridge.Logs[1] == "Warning:engine adapter warning", "warning log failed");
        Require(bridge.Logs[2] == "Error:engine adapter error", "error log failed");

        var status = bridge.ReportManagedException(
            "System.InvalidOperationException",
            "managed callback failed",
            "ManagedNet10.Smoke.EngineNode.OnReady",
            out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(DiagnosticsSmoke));
        Require(bridge.LastManagedException == "System.InvalidOperationException:managed callback failed:ManagedNet10.Smoke.EngineNode.OnReady",
            "managed exception diagnostic failed");

        status = bridge.ReportManagedException(
            "System.InvalidOperationException",
            "",
            "ManagedNet10.Smoke.EngineNode.OnReady",
            out message);
        Require(status == HostBridgeStatus.InvalidArgument && message != null, "incomplete managed exception diagnostic failed");

        bridge.FailNextCreate("host create failed");
        ExpectHostBridgeException(static state => HostObject.Create((MockHostBridge)state!, "Mock.Node", "Failure"),
            bridge,
            HostBridgeStatus.HostFailure);
    }

    private static void ExpectHostBridgeException(Action<object?> action, object? state, HostBridgeStatus expectedStatus)
    {
        try
        {
            action(state);
        }
        catch (Exception ex)
        {
            if (ex is HostBridgeException hostBridgeException && hostBridgeException.Status == expectedStatus)
            {
                return;
            }
        }

        throw new InvalidOperationException("expected HostBridgeException");
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}

internal sealed class EngineNode : IDisposable
{
    private readonly HostObject _hostObject;
    private readonly MockHostBridge _bridge;

    private EngineNode(MockHostBridge bridge, HostObject hostObject, string name)
    {
        _bridge = bridge;
        _hostObject = hostObject;
        Name = name;
    }

    public string Name { get; }

    public ulong Handle => _hostObject.Handle;

    public bool Active
    {
        get => GetProperty(nameof(Active)).AsBool();
        set => SetProperty(nameof(Active), HostValue.FromBool(value));
    }

    public int Health
    {
        get => GetProperty(nameof(Health)).AsInt32();
        set => SetProperty(nameof(Health), HostValue.FromInt32(value));
    }

    public float Speed
    {
        get => GetProperty(nameof(Speed)).AsFloat32();
        set => SetProperty(nameof(Speed), HostValue.FromFloat32(value));
    }

    public HostVector3 Position
    {
        get => GetProperty(nameof(Position)).AsVector3();
        set => SetProperty(nameof(Position), HostValue.FromVector3(value));
    }

    public static EngineNode Create(MockHostBridge bridge, string name)
    {
        return new EngineNode(bridge, HostObject.Create(bridge, "MockEngine.Node", name), name);
    }

    public HostEventSubscription OnReady(Action callback)
    {
        EnsureAlive();
        return _hostObject.Subscribe("Ready", callback);
    }

    public HostVector3 MoveBy(HostVector3 delta)
    {
        EnsureAlive();
        var status = _bridge.InvokeCommand(Handle, "MoveBy", [HostValue.FromVector3(delta)], out var result, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(EngineNode));
        return result.AsVector3();
    }

    public int Damage(int amount)
    {
        EnsureAlive();
        var status = _bridge.InvokeCommand(Handle, "Damage", [HostValue.FromInt32(amount)], out var result, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(EngineNode));
        return result.AsInt32();
    }

    public void EnsureAlive()
    {
        _hostObject.EnsureAlive();
    }

    public void Dispose()
    {
        _hostObject.Dispose();
    }

    private HostValue GetProperty(string propertyName)
    {
        EnsureAlive();
        var status = _bridge.GetProperty(Handle, propertyName, out var value, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(EngineNode));
        return value;
    }

    private void SetProperty(string propertyName, HostValue value)
    {
        EnsureAlive();
        var status = _bridge.SetProperty(Handle, propertyName, value, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(EngineNode));
    }
}

internal enum HostBridgeStatus
{
    Ok = 0,
    InvalidArgument = 1,
    UnsupportedVersion = 2,
    MissingCapability = 3,
    HostFailure = 4,
    ObjectDisposed = 5,
    ReentrantCall = 6,
    ManagedException = 7,
}

internal enum HostLogLevel
{
    Trace = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
}

internal enum HostValueKind
{
    Null = 0,
    Bool = 1,
    Int32 = 2,
    Float32 = 3,
    String = 5,
    Vector3 = 7,
}

internal readonly record struct HostVector3(float X, float Y, float Z);

internal readonly struct HostValue
{
    private HostValue(HostValueKind kind, bool boolValue, int int32Value, float float32Value, string? stringValue, HostVector3 vector3Value)
    {
        Kind = kind;
        BoolValue = boolValue;
        Int32Value = int32Value;
        Float32Value = float32Value;
        StringValue = stringValue;
        Vector3Value = vector3Value;
    }

    public HostValueKind Kind { get; }

    public bool BoolValue { get; }

    public int Int32Value { get; }

    public float Float32Value { get; }

    public string? StringValue { get; }

    public HostVector3 Vector3Value { get; }

    public static HostValue FromBool(bool value) => new(HostValueKind.Bool, value, 0, 0.0f, null, default);

    public static HostValue FromInt32(int value) => new(HostValueKind.Int32, false, value, 0.0f, null, default);

    public static HostValue FromFloat32(float value) => new(HostValueKind.Float32, false, 0, value, null, default);

    public static HostValue FromString(string value) => new(HostValueKind.String, false, 0, 0.0f, value, default);

    public static HostValue FromVector3(HostVector3 value) => new(HostValueKind.Vector3, false, 0, 0.0f, null, value);

    public bool AsBool()
    {
        RequireKind(HostValueKind.Bool);
        return BoolValue;
    }

    public int AsInt32()
    {
        RequireKind(HostValueKind.Int32);
        return Int32Value;
    }

    public float AsFloat32()
    {
        RequireKind(HostValueKind.Float32);
        return Float32Value;
    }

    public HostVector3 AsVector3()
    {
        RequireKind(HostValueKind.Vector3);
        return Vector3Value;
    }

    private void RequireKind(HostValueKind expected)
    {
        if (Kind != expected)
        {
            throw new HostBridgeException(HostBridgeStatus.InvalidArgument, "host value kind mismatch");
        }
    }
}

internal sealed class HostBridgeException : InvalidOperationException
{
    public HostBridgeException(HostBridgeStatus status, string message)
        : base(message)
    {
        Status = status;
    }

    public HostBridgeStatus Status { get; }
}

internal static class HostBridgeStatusConverter
{
    public static void ThrowIfFailed(HostBridgeStatus status, string? message, string objectName)
    {
        if (status == HostBridgeStatus.Ok)
        {
            return;
        }

        if (status == HostBridgeStatus.ObjectDisposed)
        {
            throw new ObjectDisposedException(objectName, message);
        }

        throw new HostBridgeException(status, message ?? "host bridge call failed");
    }
}

internal sealed class HostObject : IDisposable
{
    private readonly MockHostBridge _bridge;
    private bool _disposed;

    private HostObject(MockHostBridge bridge, ulong handle)
    {
        _bridge = bridge;
        Handle = handle;
    }

    public ulong Handle { get; }

    public bool IsAlive
    {
        get
        {
            EnsureNotDisposed();
            var status = _bridge.IsHandleAlive(Handle, out var alive, out var message);
            HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostObject));
            return alive;
        }
    }

    public static HostObject Create(MockHostBridge bridge, string typeName, string debugName)
    {
        var status = bridge.CreateHandle(typeName, debugName, out var handle, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostObject));
        return new HostObject(bridge, handle);
    }

    public HostEventSubscription Subscribe(string eventName, Action callback)
    {
        EnsureAlive();
        var status = _bridge.SubscribeEvent(Handle, eventName, callback, out var token, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostEventSubscription));
        return new HostEventSubscription(_bridge, token);
    }

    public void EnsureAlive()
    {
        if (!IsAlive)
        {
            throw new ObjectDisposedException(nameof(HostObject), "host object handle is no longer alive");
        }
    }

    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        var status = _bridge.ReleaseHandle(Handle, out var message);
        _disposed = true;
        if (status != HostBridgeStatus.ObjectDisposed)
        {
            HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostObject));
        }
    }

    private void EnsureNotDisposed()
    {
        if (_disposed)
        {
            throw new ObjectDisposedException(nameof(HostObject), "host object wrapper has been disposed");
        }
    }
}

internal sealed class HostEventSubscription : IDisposable
{
    private readonly MockHostBridge _bridge;
    private bool _disposed;

    public HostEventSubscription(MockHostBridge bridge, ulong token)
    {
        _bridge = bridge;
        Token = token;
    }

    public ulong Token { get; }

    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        var status = _bridge.UnsubscribeEvent(Token, out var message);
        _disposed = true;
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostEventSubscription));
    }
}

internal static class HostDispatcher
{
    public static void PumpMainThread(MockHostBridge bridge)
    {
        var status = bridge.PumpMainThread(out _, out var message);
        HostBridgeStatusConverter.ThrowIfFailed(status, message, nameof(HostDispatcher));
    }
}

internal sealed class MockHostBridge
{
    private readonly List<HandleRecord> _handles = [];
    private readonly List<EventSubscriptionRecord> _subscriptions = [];
    private readonly Queue<Action> _dispatcher = new();
    private ulong _nextHandle = 100;
    private ulong _nextSubscription = 1;
    private ulong _lastSubscription;
    private string? _nextCreateFailure;
    private string? _nextSetPropertyFailureName;
    private string? _nextSetPropertyFailureMessage;

    public List<string> Logs { get; } = [];

    public string LastManagedException { get; private set; } = "";

    public HostBridgeStatus CreateHandle(string typeName, string debugName, out ulong handle, out string? message)
    {
        if (_nextCreateFailure != null)
        {
            handle = 0;
            message = _nextCreateFailure;
            _nextCreateFailure = null;
            return HostBridgeStatus.HostFailure;
        }

        if (string.IsNullOrEmpty(typeName))
        {
            handle = 0;
            message = "host object type is required";
            return HostBridgeStatus.InvalidArgument;
        }

        handle = _nextHandle++;
        _handles.Add(new HandleRecord(handle, typeName, debugName));
        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus IsHandleAlive(ulong handle, out bool alive, out string? message)
    {
        var record = FindHandle(handle);
        alive = record != null && record.Alive && record.RefCount > 0;
        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus ReleaseHandle(ulong handle, out string? message)
    {
        var record = FindHandle(handle);
        if (record == null)
        {
            message = "host handle is unknown";
            return HostBridgeStatus.ObjectDisposed;
        }

        if (record.RefCount > 0)
        {
            record.RefCount--;
        }
        if (record.RefCount == 0)
        {
            record.Alive = false;
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus NotifyDestroyed(ulong handle)
    {
        var record = FindHandle(handle);
        if (record == null)
        {
            return HostBridgeStatus.ObjectDisposed;
        }

        record.Alive = false;
        record.RefCount = 0;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus SubscribeEvent(ulong sourceHandle, string eventName, Action callback, out ulong token, out string? message)
    {
        token = 0;
        var source = FindHandle(sourceHandle);
        if (source == null || !source.Alive || source.RefCount == 0)
        {
            message = "event source handle is no longer alive";
            return HostBridgeStatus.ObjectDisposed;
        }

        token = _nextSubscription++;
        _lastSubscription = token;
        _subscriptions.Add(new EventSubscriptionRecord(token, sourceHandle, eventName, callback));
        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus UnsubscribeEvent(ulong token, out string? message)
    {
        var subscription = FindSubscription(token);
        if (subscription != null)
        {
            subscription.Active = false;
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus TriggerEvent(ulong token)
    {
        var subscription = FindSubscription(token);
        if (subscription == null || !subscription.Active)
        {
            return HostBridgeStatus.Ok;
        }

        var source = FindHandle(subscription.SourceHandle);
        if (source == null || !source.Alive || source.RefCount == 0)
        {
            return HostBridgeStatus.ObjectDisposed;
        }

        _dispatcher.Enqueue(subscription.Callback);
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus GetProperty(ulong handle, string propertyName, out HostValue value, out string? message)
    {
        value = default;
        var record = FindLiveHandle(handle, out message);
        if (record == null)
        {
            return HostBridgeStatus.ObjectDisposed;
        }

        if (propertyName == nameof(EngineNode.Active))
        {
            value = HostValue.FromBool(record.Active);
        }
        else if (propertyName == nameof(EngineNode.Health))
        {
            value = HostValue.FromInt32(record.Health);
        }
        else if (propertyName == nameof(EngineNode.Speed))
        {
            value = HostValue.FromFloat32(record.Speed);
        }
        else if (propertyName == nameof(EngineNode.Position))
        {
            value = HostValue.FromVector3(record.Position);
        }
        else if (propertyName == nameof(EngineNode.Name))
        {
            value = HostValue.FromString(record.DebugName);
        }
        else
        {
            message = "host property is not supported";
            return HostBridgeStatus.InvalidArgument;
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus SetProperty(ulong handle, string propertyName, HostValue value, out string? message)
    {
        var record = FindLiveHandle(handle, out message);
        if (record == null)
        {
            return HostBridgeStatus.ObjectDisposed;
        }

        if (_nextSetPropertyFailureName == propertyName)
        {
            message = _nextSetPropertyFailureMessage;
            _nextSetPropertyFailureName = null;
            _nextSetPropertyFailureMessage = null;
            return HostBridgeStatus.InvalidArgument;
        }

        if (propertyName == nameof(EngineNode.Active))
        {
            if (value.Kind != HostValueKind.Bool)
            {
                message = "active requires bool";
                return HostBridgeStatus.InvalidArgument;
            }
            record.Active = value.BoolValue;
        }
        else if (propertyName == nameof(EngineNode.Health))
        {
            if (value.Kind != HostValueKind.Int32)
            {
                message = "health requires int";
                return HostBridgeStatus.InvalidArgument;
            }
            record.Health = value.Int32Value;
        }
        else if (propertyName == nameof(EngineNode.Speed))
        {
            if (value.Kind != HostValueKind.Float32)
            {
                message = "speed requires float";
                return HostBridgeStatus.InvalidArgument;
            }
            record.Speed = value.Float32Value;
        }
        else if (propertyName == nameof(EngineNode.Position))
        {
            if (value.Kind != HostValueKind.Vector3)
            {
                message = "position requires vector";
                return HostBridgeStatus.InvalidArgument;
            }
            record.Position = value.Vector3Value;
        }
        else
        {
            message = "host property is not supported";
            return HostBridgeStatus.InvalidArgument;
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public HostBridgeStatus InvokeCommand(ulong handle, string commandName, HostValue[] args, out HostValue value, out string? message)
    {
        value = default;
        var record = FindLiveHandle(handle, out message);
        if (record == null)
        {
            return HostBridgeStatus.ObjectDisposed;
        }

        if (commandName == "MoveBy")
        {
            if (args.Length != 1 || args[0].Kind != HostValueKind.Vector3)
            {
                message = "MoveBy requires one vector argument";
                return HostBridgeStatus.InvalidArgument;
            }

            var delta = args[0].Vector3Value;
            record.Position = new HostVector3(
                record.Position.X + delta.X,
                record.Position.Y + delta.Y,
                record.Position.Z + delta.Z);
            value = HostValue.FromVector3(record.Position);
        }
        else if (commandName == "Damage")
        {
            if (args.Length != 1 || args[0].Kind != HostValueKind.Int32)
            {
                message = "Damage requires one int argument";
                return HostBridgeStatus.InvalidArgument;
            }

            record.Health -= args[0].Int32Value;
            value = HostValue.FromInt32(record.Health);
        }
        else
        {
            message = "host command is not supported";
            return HostBridgeStatus.InvalidArgument;
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public void Log(HostLogLevel level, string message)
    {
        Logs.Add(level + ":" + message);
    }

    public HostBridgeStatus ReportManagedException(string exceptionType, string message, string stackTrace, out string? errorMessage)
    {
        if (string.IsNullOrEmpty(exceptionType) || string.IsNullOrEmpty(message) || string.IsNullOrEmpty(stackTrace))
        {
            errorMessage = "managed exception diagnostic is incomplete";
            return HostBridgeStatus.InvalidArgument;
        }

        LastManagedException = exceptionType + ":" + message + ":" + stackTrace;
        errorMessage = null;
        return HostBridgeStatus.Ok;
    }

    public void TriggerLastEvent()
    {
        TriggerEvent(_lastSubscription);
    }

    public HostBridgeStatus PumpMainThread(out int executed, out string? message)
    {
        executed = 0;
        while (_dispatcher.Count > 0)
        {
            var callback = _dispatcher.Dequeue();
            executed++;
            try
            {
                callback();
            }
            catch (Exception ex)
            {
                message = ex.Message;
                return HostBridgeStatus.ManagedException;
            }
        }

        message = null;
        return HostBridgeStatus.Ok;
    }

    public void FailNextCreate(string message)
    {
        _nextCreateFailure = message;
    }

    public void FailNextSetProperty(string propertyName, string message)
    {
        _nextSetPropertyFailureName = propertyName;
        _nextSetPropertyFailureMessage = message;
    }

    private HandleRecord? FindHandle(ulong handle)
    {
        for (int i = 0; i < _handles.Count; i++)
        {
            if (_handles[i].Handle == handle)
            {
                return _handles[i];
            }
        }

        return null;
    }

    private EventSubscriptionRecord? FindSubscription(ulong token)
    {
        for (int i = 0; i < _subscriptions.Count; i++)
        {
            if (_subscriptions[i].Token == token)
            {
                return _subscriptions[i];
            }
        }

        return null;
    }

    private HandleRecord? FindLiveHandle(ulong handle, out string? message)
    {
        var record = FindHandle(handle);
        if (record == null || !record.Alive || record.RefCount == 0)
        {
            message = "host handle is no longer alive";
            return null;
        }

        message = null;
        return record;
    }

    private sealed class HandleRecord
    {
        public HandleRecord(ulong handle, string typeName, string debugName)
        {
            Handle = handle;
            TypeName = typeName;
            DebugName = debugName;
        }

        public ulong Handle { get; }

        public string TypeName { get; }

        public string DebugName { get; }

        public int RefCount { get; set; } = 1;

        public bool Alive { get; set; } = true;

        public bool Active { get; set; } = true;

        public int Health { get; set; } = 100;

        public float Speed { get; set; } = 1.0f;

        public HostVector3 Position { get; set; }
    }

    private sealed class EventSubscriptionRecord
    {
        public EventSubscriptionRecord(ulong token, ulong sourceHandle, string eventName, Action callback)
        {
            Token = token;
            SourceHandle = sourceHandle;
            EventName = eventName;
            Callback = callback;
        }

        public ulong Token { get; }

        public ulong SourceHandle { get; }

        public string EventName { get; }

        public Action Callback { get; }

        public bool Active { get; set; } = true;
    }
}
