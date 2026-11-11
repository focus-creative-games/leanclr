namespace ManagedNet10.EngineContracts;
public static class Entry
{
    private static int initialized, ticks, stopped;
    public static void Initialize()
    {
        if (++initialized != 1) throw new InvalidOperationException("duplicate initialize");
    }
    public static void Tick(float delta)
    {
        if (initialized != 1 || stopped != 0) throw new InvalidOperationException("invalid lifecycle");
        if (delta < 0) throw new InvalidOperationException("tick failure");
        if (delta != 0.25f) throw new InvalidOperationException("float ABI");
        ++ticks;
        GC.Collect();
    }
    public static void Shutdown()
    {
        if (++stopped != 1 || ticks != 2) throw new InvalidOperationException("invalid shutdown");
    }
}
public static class BadSignature
{
    public static void Initialize() { }
    public static void Tick(double delta) { }
    public static void Shutdown() { }
}
public static class InitializeThrows
{
    public static void Initialize() => throw new InvalidOperationException("initialize failure");
    public static void Tick(float delta) { }
    public static void Shutdown() { }
}
public static class ShutdownThrows
{
    public static void Initialize() { }
    public static void Tick(float delta) { }
    public static void Shutdown() => throw new InvalidOperationException("shutdown failure");
}

public static class VarargsEntry
{
    public static void Initialize(__arglist) { }
    public static void Tick(float delta) { }
    public static void Shutdown() { }
}
public static class GenericEntry<T>
{
    public static void Initialize() { }
    public static void Tick(float delta) { }
    public static void Shutdown() { }
}
