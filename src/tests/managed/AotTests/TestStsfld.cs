using System;


public class TypeStaticFields2
{
    public static byte x1 = 1;
    public static sbyte x2 = 2;
    public static bool x3 = true;
    public static byte x4 = 4;
    public static short x5 = 5;
    public static ushort x6 = 6;
    public static int x7 = 7;
    public static uint x8 = 8;
    public static long x9 = 9;
    public static ulong x10 = 10;
    public static float y1 = 1f;
    public static float y2 = 2f;
    public static double y3 = 3.0;
    public static object y4 = "a";
    public static ValueTypeSize1 s1 = new ValueTypeSize1 { x1 = 1 };
    public static ValueTypeSize2 s2 = new ValueTypeSize2 { x1 = 2 };
    public static ValueTypeSize3 s3 = new ValueTypeSize3 { x1 = 3 };
    public static ValueTypeSize4 s4 = new ValueTypeSize4 { x1 = 4 };
    public static ValueTypeSize5 s5 = new ValueTypeSize5 { x1 = 5 };
    public static ValueTypeSize8 s8 = new ValueTypeSize8 { x1 = 6 };
    public static ValueTypeSize9 s9 = new ValueTypeSize9 { x1 = 7 };
    public static ValueTypeSize16 s16 = new ValueTypeSize16 { x1 = 8 };

    public static AOT_Enum_byte e1 = AOT_Enum_byte.A;
    public static AOT_Enum_sbyte e2 = AOT_Enum_sbyte.B;
    public static AOT_Enum_short e3 = AOT_Enum_short.A;
    public static AOT_Enum_ushort e4 = AOT_Enum_ushort.A;
    public static AOT_Enum_int e5 = AOT_Enum_int.B;
    public static AOT_Enum_uint e6 = AOT_Enum_uint.A;
    public static AOT_Enum_long e7 = AOT_Enum_long.B;
    public static AOT_Enum_ulong e8 = AOT_Enum_ulong.A;
}

internal class TestStsfld
{
    [UnitTest]
    public void byte_1()
    {
        TypeStaticFields2.x1 = 11;
        Assert.Equal(11, TypeStaticFields2.x1);
        Assert.Equal(2, TypeStaticFields2.x2);
        TypeStaticFields2.x1 = 1;
    }

    [UnitTest]
    public void sbyte_1()
    {
        TypeStaticFields2.x2 = 11;
        Assert.Equal(11, TypeStaticFields2.x2);
        Assert.Equal(1, TypeStaticFields2.x1);
        Assert.True(TypeStaticFields2.x3);
        TypeStaticFields2.x2 = 2;
    }

    [UnitTest]
    public void bool_1()
    {
        TypeStaticFields2.x3 = false;
        Assert.False(TypeStaticFields2.x3);
        Assert.Equal(2, TypeStaticFields2.x2);
        Assert.Equal(4, TypeStaticFields2.x4);
        TypeStaticFields2.x3 = true;
    }

    [UnitTest]
    public void short_1()
    {
        TypeStaticFields2.x5 = 15;
        Assert.Equal(15, TypeStaticFields2.x5);
        Assert.Equal(4, TypeStaticFields2.x4);
        Assert.Equal(6, TypeStaticFields2.x6);
        TypeStaticFields2.x5 = 5;
    }

    [UnitTest]
    public void ushort_1()
    {
        TypeStaticFields2.x6 = 16;
        Assert.Equal(16, TypeStaticFields2.x6);
        Assert.Equal(5, TypeStaticFields2.x5);
        Assert.Equal(7, TypeStaticFields2.x7);
        TypeStaticFields2.x6 = 6;
    }

    [UnitTest]
    public void int_1()
    {
        TypeStaticFields2.x7 = 17;
        Assert.Equal(17, TypeStaticFields2.x7);
        Assert.Equal(6, TypeStaticFields2.x6);
        Assert.Equal(8, TypeStaticFields2.x8);
        TypeStaticFields2.x7 = 7;
    }

    [UnitTest]
    public void uint_1()
    {
        TypeStaticFields2.x8 = 18;
        Assert.Equal(18, TypeStaticFields2.x8);
        Assert.Equal(7, TypeStaticFields2.x7);
        Assert.Equal(9, TypeStaticFields2.x9);
        TypeStaticFields2.x8 = 8;
    }

    [UnitTest]
    public void long_1()
    {
        TypeStaticFields2.x9 = 19;
        Assert.Equal(19, TypeStaticFields2.x9);
        Assert.Equal(8, TypeStaticFields2.x8);
        Assert.Equal(10, TypeStaticFields2.x10);
        TypeStaticFields2.x9 = 9;
    }

    [UnitTest]
    public void ulong_1()
    {
        TypeStaticFields2.x10 = 20;
        Assert.Equal(20, TypeStaticFields2.x10);
        Assert.Equal(9, TypeStaticFields2.x9);
        Assert.Equal(1f, TypeStaticFields2.y1);
        TypeStaticFields2.x10 = 10;
    }

    [UnitTest]
    public void float_1()
    {
        TypeStaticFields2.y1 = 11f;
        Assert.Equal(11f, TypeStaticFields2.y1);
        Assert.Equal(10, TypeStaticFields2.x10);
        Assert.Equal(2f, TypeStaticFields2.y2);
        TypeStaticFields2.y1 = 1f;
    }

    [UnitTest]
    public void double_1()
    {
        TypeStaticFields2.y3 = 13;
        Assert.Equal(13, TypeStaticFields2.y3);
        Assert.Equal(2f, TypeStaticFields2.y2);
        Assert.Equal("a", TypeStaticFields2.y4);
        TypeStaticFields2.y3 = 3f;
    }

    [UnitTest]
    public void str_1()
    {
        TypeStaticFields2.y4 = "b";
        Assert.Equal("b", TypeStaticFields2.y4);
        Assert.Equal(3.0, TypeStaticFields2.y3);
        TypeStaticFields2.y4 = "a";
    }

    [UnitTest]
    public void enum_byte_1()
    {
        TypeStaticFields2.e1 = default;
        Assert.Equal(default(AOT_Enum_byte), TypeStaticFields2.e1);
        Assert.Equal(AOT_Enum_sbyte.B, TypeStaticFields2.e2);
        TypeStaticFields2.e1 = AOT_Enum_byte.A;
    }

    [UnitTest]
    public void enum_sbyte_1()
    {
        TypeStaticFields2.e2 = AOT_Enum_sbyte.A;
        Assert.Equal(AOT_Enum_sbyte.A, TypeStaticFields2.e2);
        Assert.Equal(AOT_Enum_byte.A, TypeStaticFields2.e1);
        Assert.Equal(AOT_Enum_short.A, TypeStaticFields2.e3);
        TypeStaticFields2.e2 = AOT_Enum_sbyte.B;
    }

    [UnitTest]
    public void enum_short_1()
    {
        TypeStaticFields2.e3 = AOT_Enum_short.B;
        Assert.Equal(AOT_Enum_short.B, TypeStaticFields2.e3);
        Assert.Equal(AOT_Enum_sbyte.B, TypeStaticFields2.e2);
        Assert.Equal(AOT_Enum_ushort.A, TypeStaticFields2.e4);
        TypeStaticFields2.e3 = AOT_Enum_short.A;
    }

    [UnitTest]
    public void enum_ushort_1()
    {
        TypeStaticFields2.e4 = default;
        Assert.Equal(default(AOT_Enum_ushort), TypeStaticFields2.e4);
        Assert.Equal(AOT_Enum_short.A, TypeStaticFields2.e3);
        Assert.Equal(AOT_Enum_int.B, TypeStaticFields2.e5);
        TypeStaticFields2.e4 = AOT_Enum_ushort.A;
    }

    [UnitTest]
    public void enum_int_1()
    {
        TypeStaticFields2.e5 = AOT_Enum_int.A;
        Assert.Equal(AOT_Enum_int.A, TypeStaticFields2.e5);
        Assert.Equal(AOT_Enum_ushort.A, TypeStaticFields2.e4);
        Assert.Equal(AOT_Enum_uint.A, TypeStaticFields2.e6);
        TypeStaticFields2.e5 = AOT_Enum_int.B;
    }

    [UnitTest]
    public void enum_uint_1()
    {
        TypeStaticFields2.e6 = default;
        Assert.Equal(default(AOT_Enum_uint), TypeStaticFields2.e6);
        Assert.Equal(AOT_Enum_int.B, TypeStaticFields2.e5);
        Assert.Equal(AOT_Enum_long.B, TypeStaticFields2.e7);
        TypeStaticFields2.e6 = AOT_Enum_uint.A;
    }

    [UnitTest]
    public void enum_long_1()
    {
        TypeStaticFields2.e7 = AOT_Enum_long.A;
        Assert.Equal(AOT_Enum_long.A, TypeStaticFields2.e7);
        Assert.Equal(AOT_Enum_uint.A, TypeStaticFields2.e6);
        Assert.Equal(AOT_Enum_ulong.A, TypeStaticFields2.e8);
        TypeStaticFields2.e7 = AOT_Enum_long.B;
    }

    [UnitTest]
    public void enum_ulong_1()
    {
        TypeStaticFields2.e8 = default;
        Assert.Equal(default(AOT_Enum_ulong), TypeStaticFields2.e8);
        TypeStaticFields2.e8 = AOT_Enum_ulong.A;
    }

    [UnitTest]
    public void valuetypesize_1()
    {
        TypeStaticFields2.s1 = new ValueTypeSize1 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s1.x1);
        Assert.Equal("a", TypeStaticFields2.y4);
        Assert.Equal(2, TypeStaticFields2.s2.x1);
        TypeStaticFields2.s1 = new ValueTypeSize1 { x1 = 1 };
    }

    [UnitTest]
    public void valuetypesize_2()
    {
        TypeStaticFields2.s2 = new ValueTypeSize2 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s2.x1);
        Assert.Equal(1, TypeStaticFields2.s1.x1);
        Assert.Equal(3, TypeStaticFields2.s3.x1);
        TypeStaticFields2.s2 = new ValueTypeSize2 { x1 = 2 };
    }

    [UnitTest]
    public void valuetypesize_3()
    {
        TypeStaticFields2.s3 = new ValueTypeSize3 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s3.x1);
        Assert.Equal(2, TypeStaticFields2.s2.x1);
        Assert.Equal(4, TypeStaticFields2.s4.x1);
        TypeStaticFields2.s3 = new ValueTypeSize3 { x1 = 3 };
    }

    [UnitTest]
    public void valuetypesize_4()
    {
        TypeStaticFields2.s4 = new ValueTypeSize4 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s4.x1);
        Assert.Equal(3, TypeStaticFields2.s3.x1);
        Assert.Equal(5, TypeStaticFields2.s5.x1);
        TypeStaticFields2.s4 = new ValueTypeSize4 { x1 = 4 };
    }

    [UnitTest]
    public void valuetypesize_5()
    {
        TypeStaticFields2.s5 = new ValueTypeSize5 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s5.x1);
        Assert.Equal(4, TypeStaticFields2.s4.x1);
        Assert.Equal(6, TypeStaticFields2.s8.x1);
        TypeStaticFields2.s5 = new ValueTypeSize5 { x1 = 5 };
    }

    [UnitTest]
    public void valuetypesize_8()
    {
        TypeStaticFields2.s8 = new ValueTypeSize8 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s8.x1);
        Assert.Equal(5, TypeStaticFields2.s5.x1);
        Assert.Equal(7, TypeStaticFields2.s9.x1);
        TypeStaticFields2.s8 = new ValueTypeSize8 { x1 = 6 };
    }

    [UnitTest]
    public void valuetypesize_9()
    {
        TypeStaticFields2.s9 = new ValueTypeSize9 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s9.x1);
        Assert.Equal(6, TypeStaticFields2.s8.x1);
        Assert.Equal(8, TypeStaticFields2.s16.x1);
        TypeStaticFields2.s9 = new ValueTypeSize9 { x1 = 7 };
    }

    [UnitTest]
    public void valuetypesize_16()
    {
        TypeStaticFields2.s16 = new ValueTypeSize16 { x1 = 10 };
        Assert.Equal(10, TypeStaticFields2.s16.x1);
        Assert.Equal(7, TypeStaticFields2.s9.x1);
        Assert.Equal(AOT_Enum_byte.A, TypeStaticFields2.e1);
        TypeStaticFields2.s16 = new ValueTypeSize16 { x1 = 8 };
    }
}

public class TypeThreadStaticFields2
{
    [ThreadStatic]
    public static byte x1;
    [ThreadStatic]
    public static sbyte x2;
    [ThreadStatic]
    public static bool x3;
    [ThreadStatic]
    public static byte x4;
    [ThreadStatic]
    public static short x5;
    [ThreadStatic]
    public static ushort x6;
    [ThreadStatic]
    public static int x7;
    [ThreadStatic]
    public static uint x8;
    [ThreadStatic]
    public static long x9;
    [ThreadStatic]
    public static ulong x10;
    [ThreadStatic]
    public static float y1;
    [ThreadStatic]
    public static float y2;
    [ThreadStatic]
    public static double y3;
    [ThreadStatic]
    public static object y4;
    [ThreadStatic]
    public static ValueTypeSize1 s1;
    [ThreadStatic]
    public static ValueTypeSize2 s2;
    [ThreadStatic]
    public static ValueTypeSize3 s3;
    [ThreadStatic]
    public static ValueTypeSize4 s4;
    [ThreadStatic]
    public static ValueTypeSize5 s5;
    [ThreadStatic]
    public static ValueTypeSize8 s8;
    [ThreadStatic]
    public static ValueTypeSize9 s9;
    [ThreadStatic]
    public static ValueTypeSize16 s16;

    [ThreadStatic]
    public static AOT_Enum_byte e1;
    [ThreadStatic]
    public static AOT_Enum_sbyte e2;
    [ThreadStatic]
    public static AOT_Enum_short e3;
    [ThreadStatic]
    public static AOT_Enum_ushort e4;
    [ThreadStatic]
    public static AOT_Enum_int e5;
    [ThreadStatic]
    public static AOT_Enum_uint e6;
    [ThreadStatic]
    public static AOT_Enum_long e7;
    [ThreadStatic]
    public static AOT_Enum_ulong e8;


    public static void Init()
    {
        x1 = 1;
        x2 = 2;
        x3 = true;
        x4 = 4;
        x5 = 5;
        x6 = 6;
        x7 = 7;
        x8 = 8;
        x9 = 9;
        x10 = 10;
        y1 = 1f;
        y2 = 2f;
        y3 = 3.0;
        y4 = "a";
        s1 = new ValueTypeSize1 { x1 = 1 };
        s2 = new ValueTypeSize2 { x1 = 2 };
        s3 = new ValueTypeSize3 { x1 = 3 };
        s4 = new ValueTypeSize4 { x1 = 4 };
        s5 = new ValueTypeSize5 { x1 = 5 };
        s8 = new ValueTypeSize8 { x1 = 6 };
        s9 = new ValueTypeSize9 { x1 = 7 };
        s16 = new ValueTypeSize16 { x1 = 8 };

        e1 = AOT_Enum_byte.A;
        e2 = AOT_Enum_sbyte.B;
        e3 = AOT_Enum_short.A;
        e4 = AOT_Enum_ushort.A;
        e5 = AOT_Enum_int.B;
        e6 = AOT_Enum_uint.A;
        e7 = AOT_Enum_long.B;
        e8 = AOT_Enum_ulong.A;
    }
}

internal class TestStsfldThreadstatic
{
    public TestStsfldThreadstatic()
    {
        TypeThreadStaticFields2.Init();
    }

    [UnitTest]
    public void byte_1()
    {
        TypeThreadStaticFields2.x1 = 11;
        Assert.Equal(11, TypeThreadStaticFields2.x1);
        Assert.Equal(2, TypeThreadStaticFields2.x2);
        TypeThreadStaticFields2.x1 = 1;
    }

    [UnitTest]
    public void sbyte_1()
    {
        TypeThreadStaticFields2.x2 = 11;
        Assert.Equal(11, TypeThreadStaticFields2.x2);
        Assert.Equal(1, TypeThreadStaticFields2.x1);
        Assert.True(TypeThreadStaticFields2.x3);
        TypeThreadStaticFields2.x2 = 2;
    }

    [UnitTest]
    public void bool_1()
    {
        TypeThreadStaticFields2.x3 = false;
        Assert.False(TypeThreadStaticFields2.x3);
        Assert.Equal(2, TypeThreadStaticFields2.x2);
        Assert.Equal(4, TypeThreadStaticFields2.x4);
        TypeThreadStaticFields2.x3 = true;
    }

    [UnitTest]
    public void short_1()
    {
        TypeThreadStaticFields2.x5 = 15;
        Assert.Equal(15, TypeThreadStaticFields2.x5);
        Assert.Equal(4, TypeThreadStaticFields2.x4);
        Assert.Equal(6, TypeThreadStaticFields2.x6);
        TypeThreadStaticFields2.x5 = 5;
    }

    [UnitTest]
    public void ushort_1()
    {
        TypeThreadStaticFields2.x6 = 16;
        Assert.Equal(16, TypeThreadStaticFields2.x6);
        Assert.Equal(5, TypeThreadStaticFields2.x5);
        Assert.Equal(7, TypeThreadStaticFields2.x7);
        TypeThreadStaticFields2.x6 = 6;
    }

    [UnitTest]
    public void int_1()
    {
        TypeThreadStaticFields2.x7 = 17;
        Assert.Equal(17, TypeThreadStaticFields2.x7);
        Assert.Equal(6, TypeThreadStaticFields2.x6);
        Assert.Equal(8, TypeThreadStaticFields2.x8);
        TypeThreadStaticFields2.x7 = 7;
    }

    [UnitTest]
    public void uint_1()
    {
        TypeThreadStaticFields2.x8 = 18;
        Assert.Equal(18, TypeThreadStaticFields2.x8);
        Assert.Equal(7, TypeThreadStaticFields2.x7);
        Assert.Equal(9, TypeThreadStaticFields2.x9);
        TypeThreadStaticFields2.x8 = 8;
    }

    [UnitTest]
    public void long_1()
    {
        TypeThreadStaticFields2.x9 = 19;
        Assert.Equal(19, TypeThreadStaticFields2.x9);
        Assert.Equal(8, TypeThreadStaticFields2.x8);
        Assert.Equal(10, TypeThreadStaticFields2.x10);
        TypeThreadStaticFields2.x9 = 9;
    }

    [UnitTest]
    public void ulong_1()
    {
        TypeThreadStaticFields2.x10 = 20;
        Assert.Equal(20, TypeThreadStaticFields2.x10);
        Assert.Equal(9, TypeThreadStaticFields2.x9);
        Assert.Equal(1f, TypeThreadStaticFields2.y1);
        TypeThreadStaticFields2.x10 = 10;
    }

    [UnitTest]
    public void float_1()
    {
        TypeThreadStaticFields2.y1 = 11f;
        Assert.Equal(11f, TypeThreadStaticFields2.y1);
        Assert.Equal(10, TypeThreadStaticFields2.x10);
        Assert.Equal(2f, TypeThreadStaticFields2.y2);
        TypeThreadStaticFields2.y1 = 1f;
    }

    [UnitTest]
    public void double_1()
    {
        TypeThreadStaticFields2.y3 = 13;
        Assert.Equal(13, TypeThreadStaticFields2.y3);
        Assert.Equal(2f, TypeThreadStaticFields2.y2);
        Assert.Equal("a", TypeThreadStaticFields2.y4);
        TypeThreadStaticFields2.y3 = 3f;
    }

    [UnitTest]
    public void str_1()
    {
        TypeThreadStaticFields2.y4 = "b";
        Assert.Equal("b", TypeThreadStaticFields2.y4);
        Assert.Equal(3.0, TypeThreadStaticFields2.y3);
        TypeThreadStaticFields2.y4 = "a";
    }

    [UnitTest]
    public void enum_byte_1()
    {
        TypeThreadStaticFields2.e1 = default;
        Assert.Equal(default(AOT_Enum_byte), TypeThreadStaticFields2.e1);
        Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields2.e2);
        TypeThreadStaticFields2.e1 = AOT_Enum_byte.A;
    }

    [UnitTest]
    public void enum_sbyte_1()
    {
        TypeThreadStaticFields2.e2 = AOT_Enum_sbyte.A;
        Assert.Equal(AOT_Enum_sbyte.A, TypeThreadStaticFields2.e2);
        Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields2.e1);
        Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields2.e3);
        TypeThreadStaticFields2.e2 = AOT_Enum_sbyte.B;
    }

    [UnitTest]
    public void enum_short_1()
    {
        TypeThreadStaticFields2.e3 = AOT_Enum_short.B;
        Assert.Equal(AOT_Enum_short.B, TypeThreadStaticFields2.e3);
        Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields2.e2);
        Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields2.e4);
        TypeThreadStaticFields2.e3 = AOT_Enum_short.A;
    }

    [UnitTest]
    public void enum_ushort_1()
    {
        TypeThreadStaticFields2.e4 = default;
        Assert.Equal(default(AOT_Enum_ushort), TypeThreadStaticFields2.e4);
        Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields2.e3);
        Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields2.e5);
        TypeThreadStaticFields2.e4 = AOT_Enum_ushort.A;
    }

    [UnitTest]
    public void enum_int_1()
    {
        TypeThreadStaticFields2.e5 = AOT_Enum_int.A;
        Assert.Equal(AOT_Enum_int.A, TypeThreadStaticFields2.e5);
        Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields2.e4);
        Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields2.e6);
        TypeThreadStaticFields2.e5 = AOT_Enum_int.B;
    }

    [UnitTest]
    public void enum_uint_1()
    {
        TypeThreadStaticFields2.e6 = default;
        Assert.Equal(default(AOT_Enum_uint), TypeThreadStaticFields2.e6);
        Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields2.e5);
        Assert.Equal(AOT_Enum_long.B, TypeThreadStaticFields2.e7);
        TypeThreadStaticFields2.e6 = AOT_Enum_uint.A;
    }

    [UnitTest]
    public void enum_long_1()
    {
        TypeThreadStaticFields2.e7 = AOT_Enum_long.A;
        Assert.Equal(AOT_Enum_long.A, TypeThreadStaticFields2.e7);
        Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields2.e6);
        Assert.Equal(AOT_Enum_ulong.A, TypeThreadStaticFields2.e8);
        TypeThreadStaticFields2.e7 = AOT_Enum_long.B;
    }

    [UnitTest]
    public void enum_ulong_1()
    {
        TypeThreadStaticFields2.e8 = default;
        Assert.Equal(default(AOT_Enum_ulong), TypeThreadStaticFields2.e8);
        TypeThreadStaticFields2.e8 = AOT_Enum_ulong.A;
    }

    [UnitTest]
    public void valuetypesize_1()
    {
        TypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s1.x1);
        Assert.Equal("a", TypeThreadStaticFields2.y4);
        Assert.Equal(2, TypeThreadStaticFields2.s2.x1);
        TypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 1 };
    }

    [UnitTest]
    public void valuetypesize_2()
    {
        TypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s2.x1);
        Assert.Equal(1, TypeThreadStaticFields2.s1.x1);
        Assert.Equal(3, TypeThreadStaticFields2.s3.x1);
        TypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 2 };
    }

    [UnitTest]
    public void valuetypesize_3()
    {
        TypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s3.x1);
        Assert.Equal(2, TypeThreadStaticFields2.s2.x1);
        Assert.Equal(4, TypeThreadStaticFields2.s4.x1);
        TypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 3 };
    }

    [UnitTest]
    public void valuetypesize_4()
    {
        TypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s4.x1);
        Assert.Equal(3, TypeThreadStaticFields2.s3.x1);
        Assert.Equal(5, TypeThreadStaticFields2.s5.x1);
        TypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 4 };
    }

    [UnitTest]
    public void valuetypesize_5()
    {
        TypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s5.x1);
        Assert.Equal(4, TypeThreadStaticFields2.s4.x1);
        Assert.Equal(6, TypeThreadStaticFields2.s8.x1);
        TypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 5 };
    }

    [UnitTest]
    public void valuetypesize_8()
    {
        TypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s8.x1);
        Assert.Equal(5, TypeThreadStaticFields2.s5.x1);
        Assert.Equal(7, TypeThreadStaticFields2.s9.x1);
        TypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 6 };
    }

    [UnitTest]
    public void valuetypesize_9()
    {
        TypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s9.x1);
        Assert.Equal(6, TypeThreadStaticFields2.s8.x1);
        Assert.Equal(8, TypeThreadStaticFields2.s16.x1);
        TypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 7 };
    }

    [UnitTest]
    public void valuetypesize_16()
    {
        TypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 10 };
        Assert.Equal(10, TypeThreadStaticFields2.s16.x1);
        Assert.Equal(7, TypeThreadStaticFields2.s9.x1);
        Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields2.e1);
        TypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 8 };
    }
}
