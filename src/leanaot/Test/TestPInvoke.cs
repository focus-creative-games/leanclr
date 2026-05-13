using System;
using System.Runtime.InteropServices;

/// <summary>
/// P/Invoke 覆盖：在 Emscripten 下由 <c>leanclr_test_pinvoke.js</c> 提供 wasm 导入实现，
/// 入口名须与 C# 中 <see cref="DllImportAttribute.EntryPoint"/> 一致。
/// </summary>
/// <remarks>
/// 以下为带 <see cref="UnitTestAttribute"/> 的用例；在已链接 <c>__Internal</c> 原生桩（如 aot-runner）或
/// Emscripten（<c>leanclr_test_pinvoke.js</c>）的环境下由测试运行器执行。
/// 单独运行示例：<c>simple-aot ... -e WasmPInvokeVerify::UnitTest_AddI32 Test</c>。
/// </remarks>
// Blittable struct passed by value; native layout uses LeanAOT instance fields __field_0 / __field_1 in order.
[StructLayout(LayoutKind.Sequential)]
public struct LeanClrPinvokeTestPair
{
    public int First;
    public int Second;
}

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate int LeanClrPinvokeBinaryOp(int a, int b);

// Minimal SafeHandle subclass for P/Invoke handle field extraction tests.
public sealed class LeanClrPinvokeTestHandle : SafeHandle
{
    private LeanClrPinvokeTestHandle()
        : base(IntPtr.Zero, ownsHandle: true)
    {
    }

    public static LeanClrPinvokeTestHandle FromValue(int value)
    {
        var h = new LeanClrPinvokeTestHandle();
        h.SetHandle((IntPtr)value);
        return h;
    }

    public override bool IsInvalid => false;

    protected override bool ReleaseHandle() => true;
}

public static class TestPInvokeNative
{
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_add_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int AddI32(int a, int b);

    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_mul_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int MulI32(int a, int b);

    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_neg_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern int NegI32(int x);

    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_is_nonzero_i32", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsNonZeroI32(int x);

    /// <summary>入参为 UTF-8（由运行时代码从 UTF-16 转换），返回字节数（不含结尾 0）。</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_utf8_byte_len", CallingConvention = CallingConvention.Cdecl)]
    public static extern int Utf8ByteLen(string s);

    /// <summary>返回 null。</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_return_null_utf8", CallingConvention = CallingConvention.Cdecl)]
    public static extern string ReturnNullUtf8(string s);

    /// <summary><paramref name="arr"/> 为元素区首地址（int32_t*），与 <paramref name="count"/> 一起求前 count 项之和。</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_sum_int_range", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SumIntRange(int[] arr, int count);

    /// <summary>Native computes <c>First * Second + Second</c> (uses blittable layout matching LeanAOT field names <c>__field_0</c>/<c>__field_1</c>).</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_struct_pair_mul_add", CallingConvention = CallingConvention.Cdecl)]
    public static extern int StructPairMulAdd(LeanClrPinvokeTestPair p);

    /// <summary>Invokes native code which calls the marshaled function pointer (<c>void*</c> cdecl).</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_invoke_binary_op", CallingConvention = CallingConvention.Cdecl)]
    public static extern int InvokeBinaryOp(LeanClrPinvokeBinaryOp op, int a, int b);

    /// <summary>Receives raw handle value from <see cref="SafeHandle"/> plus ten.</summary>
    [DllImport("__Internal", EntryPoint = "leanclr_pinvoke_safe_handle_add_ten", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SafeHandleAddTen(LeanClrPinvokeTestHandle h);
}

public class WasmPInvokeVerify
{
    [UnitTest]
    public void UnitTest_AddI32()
    {
        Assert.Equal(7, TestPInvokeNative.AddI32(3, 4));
    }

    [UnitTest]
    public void UnitTest_IntegerArithmetic()
    {
        Assert.Equal(42, TestPInvokeNative.MulI32(6, 7));
        Assert.Equal(-9, TestPInvokeNative.NegI32(9));
        Assert.Equal(false, TestPInvokeNative.IsNonZeroI32(0));
        Assert.Equal(true, TestPInvokeNative.IsNonZeroI32(-1));
    }

    [UnitTest]
    public void UnitTest_Utf8StringMarshalling()
    {
        Assert.Equal(0, TestPInvokeNative.Utf8ByteLen(""));
        Assert.Equal(5, TestPInvokeNative.Utf8ByteLen("abcde"));
        Assert.Equal(6, TestPInvokeNative.Utf8ByteLen("你好"));
        Assert.Null(TestPInvokeNative.ReturnNullUtf8(""));
    }

    [UnitTest]
    public void UnitTest_SumIntRange()
    {
        int[] xs = new int[] { 10, 20, 30, 40 };
        Assert.Equal(100, TestPInvokeNative.SumIntRange(xs, 4));
        Assert.Equal(30, TestPInvokeNative.SumIntRange(xs, 2));
    }

    [UnitTest]
    public void UnitTest_BlittableStruct()
    {
        var pair = new LeanClrPinvokeTestPair { First = 3, Second = 4 };
        Assert.Equal(16, TestPInvokeNative.StructPairMulAdd(pair));
    }

    [UnitTest]
    public void UnitTest_DelegateCallback()
    {
        LeanClrPinvokeBinaryOp mul = (a, b) => a * b;
        Assert.Equal(12, TestPInvokeNative.InvokeBinaryOp(mul, 3, 4));
    }

    [UnitTest]
    public void UnitTest_SafeHandle()
    {
        using (LeanClrPinvokeTestHandle h = LeanClrPinvokeTestHandle.FromValue(32))
        {
            Assert.Equal(42, TestPInvokeNative.SafeHandleAddTen(h));
        }
    }
}
