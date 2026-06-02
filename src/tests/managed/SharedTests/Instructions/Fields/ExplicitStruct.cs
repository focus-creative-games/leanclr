using System.Runtime.InteropServices;

namespace Tests.Instruments.Fields
{
    [StructLayout(LayoutKind.Explicit)]
    public struct ExplicitStruct
    {
        [FieldOffset(0)] public int x;
        [FieldOffset(0)] public long y;
        [FieldOffset(0)] public float a;
        [FieldOffset(0)] public double b;
    }
}