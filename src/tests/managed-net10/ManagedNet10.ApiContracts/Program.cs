namespace ManagedNet10.ApiContracts;
internal static class Program
{
    private static void Check(bool condition, string message) { if (!condition) throw new Exception(message); }
    public static void Main() { TestArrayLayout(); TestArrayInterfaces(); TestSpanAndCollections(); TestWorkerThreadRejected(); TestStructReturn(); TestVarargsRejected(); }
    public static void TestArrayLayout()
    {
        var values = new int[2, 3];
        Check(values[0, 0] == 0 && values[1, 2] == 0, "array bounds overlap element data");
        values[1, 2] = 42;
        Check(values[1, 2] == 42 && values.GetLength(1) == 3, "multidimensional array access");
        var copy = (int[,])values.Clone();
        GC.Collect();
        Check(copy[1, 2] == 42 && copy.GetLowerBound(0) == 0, "array clone layout");
        var nonzero = Array.CreateInstance(typeof(int), new[] { 2, 3 }, new[] { -1, 4 });
        nonzero.SetValue(77, -1, 6);
        Check((int)nonzero.GetValue(-1, 6)! == 77, "nonzero array lower bounds");
        try { _ = values[2, 0]; throw new Exception("bounds check accepted invalid index"); }
        catch (IndexOutOfRangeException) { }
    }
    public static void TestArrayInterfaces()
    {
        IList<int> values = new[] { 1, 2, 3 };
        Check(values.Count == 3 && values.Contains(2), "array generic interfaces");
        values[1] = 42;
        Check(values[1] == 42, "array interface assignment");
        try { values.Add(4); throw new Exception("fixed-size array grew"); }
        catch (NotSupportedException) { }
    }
    public static void TestSpanAndCollections()
    {
        Span<int> values = stackalloc int[4];
        values.Fill(7); values[2] = 42;
        var list = new List<int>(values.ToArray());
        var map = new Dictionary<string, int> { ["answer"] = list[2] };
        Check(map["answer"] == 42 && string.Join(",", list) == "7,7,42,7", "span/collection/string contract");
        Check(typeof(List<int>).GetGenericArguments()[0] == typeof(int), "basic generic reflection");
    }
    public static void TestWorkerThreadRejected()
    {
        bool ran = false;
        var thread = new Thread(() => ran = true);
        try { thread.Start(); throw new Exception("worker start incorrectly succeeded"); }
        catch (NotSupportedException) { }
        Check(!ran, "worker executed synchronously");
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    private static (long First, long Second) Pair(long value) => (value, value + 1);
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    private static int VarargsTarget(__arglist) => new ArgIterator(__arglist).GetRemainingCount();
    public static void TestVarargsRejected()
    {
        try { _ = VarargsTarget(__arglist()); throw new Exception("varargs incorrectly succeeded"); }
        catch (NotSupportedException) { }
    }
    public static void TestStructReturn()
    {
        var result = Pair(41);
        Check(result.First == 41 && result.Second == 42, "overlapping struct return buffer");
    }
}
