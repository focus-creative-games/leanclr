using System.Runtime.CompilerServices;
namespace ManagedNet10.GcContracts;
internal static class Program
{
    private sealed class Box { public object? Reference; public int Value = 42; }
    [InlineArray(4)] private struct References { private object? element; }
    private sealed class Buffer { public References Values; }
    private sealed class Finalizable { ~Finalizable() { finalized++; } }
    private static int finalized;
    private static Buffer? buffer;
    private static void Check(bool value, string message) { if (!value) throw new Exception(message); }
    public static void Main() { TestLiveObject(); TestInlineArrayReferences(); TestDeadEphemeronCycle(); TestLiveEphemeronChain(); TestFinalizer(); TestInteriorByRef(); TestLargeInteriorByRef(); TestValueTypeReferences(); TestExceptionRoots(); TestFinalizerClosure(); TestStaticConstructorRoots(); TestNestedCatchInFinally(); TestThrowAtTryEnd(); TestByRefReplacement(); TestReflectionArguments(); }
    public static void TestLiveObject()
    {
        var value = new Box { Reference = new object() };
        for (int i = 0; i < 10; i++) GC.Collect();
        Check(value.Value == 42 && value.Reference != null, "live references were collected");
        GC.KeepAlive(value);
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static void FillBuffer()
    {
        buffer = new Buffer();
        for (int i = 0; i < 4; i++) buffer.Values[i] = new Box { Value = 100 + i };
    }
    public static void TestInlineArrayReferences()
    {
        FillBuffer();
        for (int i = 0; i < 4; i++) GC.Collect();
        for (int i = 0; i < 4; i++) Check(((Box)buffer!.Values[i]!).Value == 100 + i, "inline array GC bitmap lost an element");
        buffer = null;
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static WeakReference MakeCycle(ConditionalWeakTable<object, Box> table)
    {
        object key = new object();
        table.Add(key, new Box { Reference = key });
        return new WeakReference(key);
    }
    public static void TestDeadEphemeronCycle()
    {
        var table = new ConditionalWeakTable<object, Box>();
        var weak = MakeCycle(table);
        GC.Collect(); GC.Collect();
        Check(!weak.IsAlive, "unrooted dependent-handle cycle survived");
        GC.KeepAlive(table);
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static (object, WeakReference) MakeChain(ConditionalWeakTable<object, Box> table)
    {
        object first = new object(), second = new object();
        var last = new Box();
        table.Add(first, new Box { Reference = second });
        table.Add(second, last);
        return (first, new WeakReference(last));
    }
    public static void TestLiveEphemeronChain()
    {
        var table = new ConditionalWeakTable<object, Box>();
        var (first, last) = MakeChain(table);
        GC.Collect();
        Check(last.IsAlive, "dependent-handle closure lost a reachable value");
        GC.KeepAlive(first); GC.KeepAlive(table);
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static WeakReference MakeFinalizable() => new WeakReference(new Finalizable());
    public static void TestFinalizer()
    {
        finalized = 0;
        var weak = MakeFinalizable();
        GC.Collect(); GC.WaitForPendingFinalizers(); GC.Collect();
        Check(finalized == 1 && !weak.IsAlive, "finalization or weak clearing failed");
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static ref int MakeInterior() => ref new Box().Value;
    public static void TestInteriorByRef()
    {
        ref int value = ref MakeInterior();
        GC.Collect();
        Check(value == 42, "interior byref owner was collected");
        value = 43;
        GC.Collect();
        Check(value == 43, "interior byref did not remain valid");
    }

    [MethodImpl(MethodImplOptions.NoInlining)] private static ref byte MakeLargeInterior()
    {
        var bytes = new byte[32000];
        bytes[12345] = 42;
        return ref bytes[12345];
    }
    public static void TestLargeInteriorByRef()
    {
        ref byte value = ref MakeLargeInterior();
        for (int i = 0; i < 32; i++) { GC.Collect(); Check(value == 42, "large-object interior root was lost"); }
    }
    private struct ReferencePair { public object First; public object Second; }
    [MethodImpl(MethodImplOptions.NoInlining)] private static ReferencePair MakePair() => new ReferencePair { First = new Box(), Second = new Box() };
    public static void TestValueTypeReferences()
    {
        var pair = MakePair();
        for (int i = 0; i < 32; i++) GC.Collect();
        Check(((Box)pair.First).Value == 42 && ((Box)pair.Second).Value == 42, "struct references were lost");
    }
    private sealed class PayloadException : Exception { public readonly Box Payload = new Box(); }
    private static bool Filter(Exception exception)
    {
        try { throw new Exception("nested"); } catch { }
        GC.Collect();
        return exception is PayloadException payload && payload.Payload.Value == 42;
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static void ThrowPayload() => throw new PayloadException();
    public static void TestExceptionRoots()
    {
        int finalizers = 0;
        try
        {
            try { ThrowPayload(); }
            catch (PayloadException exception) when (Filter(exception))
            {
                try { throw new Exception("replace current exception"); } catch { }
                GC.Collect();
                Check(exception.Payload.Value == 42, "exception root was lost in handler");
                throw;
            }
            finally { GC.Collect(); finalizers++; }
        }
        catch (PayloadException exception) { Check(exception.Payload.Value == 42, "rethrow lost original exception"); }
        Check(finalizers == 1, "finally did not run exactly once");
    }
    private static int closureFinalized;
    private sealed class CollectingFinalizer
    {
        private readonly Box payload = new Box { Reference = new Box() };
        ~CollectingFinalizer()
        {
            GC.Collect();
            if (payload.Value == 42 && ((Box)payload.Reference!).Value == 42) closureFinalized++;
        }
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static void MakeFinalizerBatch()
    {
        for (int i = 0; i < 12; i++) _ = new CollectingFinalizer();
    }
    public static void TestFinalizerClosure()
    {
        closureFinalized = 0;
        MakeFinalizerBatch();
        GC.Collect(); GC.WaitForPendingFinalizers(); GC.Collect(); GC.WaitForPendingFinalizers();
        Check(closureFinalized == 12, "finalizer queue or transitive payload was collected");
    }
    private static class CollectingInitializer
    {
        public static readonly int Value;
        static CollectingInitializer() { GC.Collect(); Value = 42; }
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static int ReadInitializer() => CollectingInitializer.Value;
    [MethodImpl(MethodImplOptions.NoInlining)] private static void Consume(Box box, int value) => Check(box.Value == value, "cctor lost a live stack root");
    public static void TestStaticConstructorRoots() => Consume(new Box(), ReadInitializer());

    public static void TestNestedCatchInFinally()
    {
        int count = 0;
        try { count++; }
        finally
        {
            try { count += 10; throw new Exception(); }
            catch { count += 20; }
            finally { count += 100; }
            count += 1000;
        }
        Check(count == 1131, "catch nested in finally resumed the handled throw");
    }
    [MethodImpl(MethodImplOptions.NoInlining)] private static void ThrowLastCall() => throw new PayloadException();
    public static void TestThrowAtTryEnd()
    {
        int count = 0;
        try { ThrowLastCall(); }
        catch (PayloadException) { count++; }
        finally { count += 10; }
        Check(count == 11, "call at try boundary escaped its handler");
    }

    [MethodImpl(MethodImplOptions.NoInlining)] private static void ReplaceAndCollect(ref Box? value)
    {
        value = new Box { Reference = new Box() };
        GC.Collect();
        Check(value.Value == 42 && ((Box)value.Reference!).Value == 42, "callee lost replacement through a byref argument");
    }
    public static void TestByRefReplacement()
    {
        Box? value = null;
        ReplaceAndCollect(ref value);
        GC.Collect();
        Check(value!.Value == 42, "byref replacement did not survive return");
    }
    public static void CollectArgument(object value)
    {
        GC.Collect();
        Check(((Box)value).Value == 42, "reflection invocation lost native argument roots");
    }
    public static void TestReflectionArguments()
    {
        var method = typeof(Program).GetMethod(nameof(CollectArgument))!;
        for (int i = 0; i < 8; i++) method.Invoke(null, new object[] { new Box() });
    }
}
