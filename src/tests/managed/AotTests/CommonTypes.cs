using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Explicit, Pack = 8, Size = 919816)]
public unsafe struct CSDT_DESK_PLAYERINFO
{
    public const int astCampPlayerInfo_length = 32;

    [FieldOffset(0)] internal fixed byte astCampPlayerInfo_bytes[astCampPlayerInfo_length * 28744];
    [FieldOffset(919808)] public int dwPlayerNum;
    [FieldOffset(919812)] public byte bCampNum;
}

[StructLayout(LayoutKind.Explicit, Size = 32)]
public struct StructWithExplicitLayout1
{
    [FieldOffset(0)] public byte x1;
    [FieldOffset(1)] public byte x2;
    [FieldOffset(2)] public byte x3;
    [FieldOffset(3)] public byte x4;
    [FieldOffset(4)] public byte x5;
}

[StructLayout(LayoutKind.Explicit, Pack = 2)]
public struct StructWithExplicitLayout2
{
    [FieldOffset(0)] public int x1;
    [FieldOffset(4)] public int x5;
    [FieldOffset(8)] public byte x6;
}

[StructLayout(LayoutKind.Sequential, Pack = 2)]
public struct StructWithExplicitLayout3
{
    public byte x0;
    public int x1;
    public byte x2;
    public int x5;
}

[StructLayout(LayoutKind.Explicit, Size = 32)]
public class ClassWithExplicitLayout1
{
    [FieldOffset(0)] public byte x1;
    [FieldOffset(1)] public byte x2;
    [FieldOffset(2)] public byte x3;
    [FieldOffset(3)] public byte x4;
    [FieldOffset(4)] public byte x5;
}

[StructLayout(LayoutKind.Explicit, Pack = 2)]
public class ClassWithExplicitLayout2
{
    [FieldOffset(0)] public int x1;
    [FieldOffset(4)] public int x5;
}

[StructLayout(LayoutKind.Sequential, Pack = 2)]
public class ClassWithExplicitLayout3
{
    public int x1;
    public byte x2;
    public int x5;
}
