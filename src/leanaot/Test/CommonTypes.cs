using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


public enum AOT_Enum_byte : byte
{
    A = 1,
    B = 2,
    C = 3,
}

public enum AOT_Enum_sbyte : sbyte
{
    A = -1,
    B = 1,
}

public enum AOT_Enum_short : short
{
    A = -1,
    B = 1,
}

public enum AOT_Enum_ushort : ushort
{
    A = 1,
}

public enum AOT_Enum_int : int
{
    A = -1,
    B = 1,
    C = 10,
    D = 1000,
}

public enum AOT_Enum_uint : uint
{
    A = 1,
}

public enum AOT_Enum_long : long
{
    A = -0x1122334455667788,
    B = 0x1122334455667788,
}


public enum AOT_Enum_ulong : ulong
{
    A = 0x1122334455667788,
}
public enum AOT_Enum_byte2 : byte
{
    A = 1,
    B = 2,
    C = 3,
}

public enum AOT_Enum_int2 : int
{
    B = 1,
    C = 10,
    D = 1000,
}

public struct ValueTypeSize0
{
    public unsafe void InitAs1()
    {
        ValueTypeSize0 v = default;
        *(byte*)&v = 1;
        this = v;
    }

    public unsafe int GetValue()
    {
        ValueTypeSize0 v = this;
        return *(byte*)&v;
    }
}

public struct ValueTypeSize1
{
    public byte x1;

    public override string ToString()
    {
        return $"x1={x1}";
    }
}

public struct ValueTypeSize2
{
    public byte x1;
    public byte x2;
}

public struct ValueTypeSize3
{
    public byte x1;
    public byte x2;
    public byte x3;
}

public struct ValueTypeSize4
{
    public int x1;
}

public struct ValueTypeSize5
{
    public byte x1;
    public byte x2;
    public byte x3;
    public byte x4;
    public byte x5;
}

public struct ValueTypeSize8
{
    public long x1;
}

public struct ValueTypeSize9
{
    public byte x1;
    public byte x2;
    public byte x3;
    public byte x4;
    public byte x5;
    public byte x6;
    public byte x7;
    public byte x8;
    public byte x9;
}

public struct ValueTypeSize16
{
    public long x1;
    public long x2;
}

public struct ValueTypeSize20
{
    public int x1;
    public int x2;
    public int x3;
    public int x4;
    public int x5;
}

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


public struct Vector2
{
    public float x;
    public float y;

    public Vector2(float x, float y)
    {
        this.x = x;
        this.y = y;
    }
}

public struct Vector3
{
    public float x;
    public float y;
    public float z;

    public Vector3(float x, float y, float z)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }
}

public struct Vector4
{
    public float x;
    public float y;
    public float z;
    public float w;

    public Vector4(float x, float y, float z, float w)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }
}
