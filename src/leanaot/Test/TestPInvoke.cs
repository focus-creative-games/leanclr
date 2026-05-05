using System.Runtime.InteropServices;

/// <summary>
/// P/Invoke 覆盖：在 Emscripten 下由 <c>leanclr_test_pinvoke.js</c> 提供 wasm 导入实现，
/// 入口名须与 C# 中 <see cref="DllImportAttribute.EntryPoint"/> 一致。
/// </summary>
/// <remarks>
/// 非 Emscripten 目标下 LeanAOT 生成的 P/Invoke 体会返回未实现，因此不要给本文件中的检查加
/// <see cref="UnitTestAttribute"/>，否则会破坏本地 MSVC 运行 <c>App::Main</c> 的全量测试。
/// WebAssembly 构建完成后请使用：
/// <c>simple-aot ... -e WasmPInvokeVerify::Main Test</c>
/// </remarks>
public static class TestPInvokeNative
{
    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_add_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int AddI32(int a, int b);

    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_mul_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int MulI32(int a, int b);

    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_neg_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int NegI32(int x);

    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_is_nonzero_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsNonZeroI32(int x);

    /// <summary>入参为 UTF-8（由运行时代码从 UTF-16 转换），返回字节数（不含结尾 0）。</summary>
    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_utf8_byte_len", CallingConvention = CallingConvention.Cdecl)]
    public static extern int Utf8ByteLen(string s);

    /// <summary>返回 null。</summary>
    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_return_null_utf8", CallingConvention = CallingConvention.Cdecl)]
    public static extern string ReturnNullUtf8(string s);

    /// <summary><paramref name="arr"/> 为元素区首地址（int32_t*），与 <paramref name="count"/> 一起求前 count 项之和。</summary>
    [DllImport("LeanClrTestPInvoke.dll", EntryPoint = "leanclr_pinvoke_sum_int_range", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SumIntRange(int[] arr, int count);
}

/// <summary>
/// 仅用于 Wasm 端到端验证（见文件头说明）；与 <see cref="App"/> 的全量单元测试入口分离。
/// </summary>
public static class WasmPInvokeVerify
{
    public static void Main()
    {
        Assert.Equal(7, TestPInvokeNative.AddI32(3, 4));
        Assert.Equal(42, TestPInvokeNative.MulI32(6, 7));
        Assert.Equal(-9, TestPInvokeNative.NegI32(9));
        Assert.Equal(false, TestPInvokeNative.IsNonZeroI32(0));
        Assert.Equal(true, TestPInvokeNative.IsNonZeroI32(-1));

        Assert.Equal(0, TestPInvokeNative.Utf8ByteLen(""));
        Assert.Equal(5, TestPInvokeNative.Utf8ByteLen("abcde"));
        Assert.Equal(6, TestPInvokeNative.Utf8ByteLen("你好"));

        Assert.Null(TestPInvokeNative.ReturnNullUtf8(""));

        int[] xs = new int[] { 10, 20, 30, 40 };
        Assert.Equal(100, TestPInvokeNative.SumIntRange(xs, 4));
        Assert.Equal(30, TestPInvokeNative.SumIntRange(xs, 2));
        Assert.Equal(0, TestPInvokeNative.SumIntRange(null, 3));

        App.s_logger("WasmPInvokeVerify: all checks passed.\n");
    }
}
