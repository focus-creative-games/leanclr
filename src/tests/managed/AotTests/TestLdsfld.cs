using System;


public class TypeStaticFields
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

internal class TestLdsfld
{
    [UnitTest]
    public void byte_1()
    {
        Assert.Equal(1, TypeStaticFields.x1);
    }

    [UnitTest]
    public void sbyte_1()
    {
        Assert.Equal(2, TypeStaticFields.x2);
    }

    [UnitTest]
    public void bool_1()
    {
        Assert.True(TypeStaticFields.x3);
    }

    [UnitTest]
    public void short_1()
    {
        Assert.Equal(5, TypeStaticFields.x5);
    }

    [UnitTest]
    public void ushort_1()
    {
        Assert.Equal(6, TypeStaticFields.x6);
    }

    [UnitTest]
    public void int_1()
    {
        Assert.Equal(7, TypeStaticFields.x7);
    }

    [UnitTest]
    public void uint_1()
    {
        Assert.Equal(8, TypeStaticFields.x8);
    }

    [UnitTest]
    public void long_1()
    {
        Assert.Equal(9, TypeStaticFields.x9);
    }

    [UnitTest]
    public void ulong_1()
    {
        Assert.Equal(10, TypeStaticFields.x10);
    }

    [UnitTest]
    public void float_1()
    {
        Assert.Equal(1f, TypeStaticFields.y1);
    }

    [UnitTest]
    public void double_1()
    {
        Assert.Equal(3, TypeStaticFields.y3);
    }

    [UnitTest]
    public void str_1()
    {
        Assert.Equal("a", TypeStaticFields.y4);
    }

    [UnitTest]
    public void enum_byte_1()
    {
        Assert.Equal(AOT_Enum_byte.A, TypeStaticFields.e1);
    }

    [UnitTest]
    public void enum_sbyte_1()
    {
        Assert.Equal(AOT_Enum_sbyte.B, TypeStaticFields.e2);
    }

    [UnitTest]
    public void enum_short_1()
    {
        Assert.Equal(AOT_Enum_short.A, TypeStaticFields.e3);
    }

    [UnitTest]
    public void enum_ushort_1()
    {
        Assert.Equal(AOT_Enum_ushort.A, TypeStaticFields.e4);
    }

    [UnitTest]
    public void enum_int_1()
    {
        Assert.Equal(AOT_Enum_int.B, TypeStaticFields.e5);
    }

    [UnitTest]
    public void enum_uint_1()
    {
        Assert.Equal(AOT_Enum_uint.A, TypeStaticFields.e6);
    }

    [UnitTest]
    public void enum_long_1()
    {
        Assert.Equal(AOT_Enum_long.B, TypeStaticFields.e7);
    }

    [UnitTest]
    public void enum_ulong_1()
    {
        Assert.Equal(AOT_Enum_ulong.A, TypeStaticFields.e8);
    }

    [UnitTest]
    public void valuetypesize_1()
    {
        Assert.Equal(1, TypeStaticFields.s1.x1);
    }

    [UnitTest]
    public void valuetypesize_2()
    {
        Assert.Equal(2, TypeStaticFields.s2.x1);
    }

    [UnitTest]
    public void valuetypesize_3()
    {
        Assert.Equal(3, TypeStaticFields.s3.x1);
    }

    [UnitTest]
    public void valuetypesize_4()
    {
        Assert.Equal(4, TypeStaticFields.s4.x1);
    }

    [UnitTest]
    public void valuetypesize_5()
    {
        Assert.Equal(5, TypeStaticFields.s5.x1);
    }

    [UnitTest]
    public void valuetypesize_8()
    {
        Assert.Equal(6, TypeStaticFields.s8.x1);
    }

    [UnitTest]
    public void valuetypesize_9()
    {
        Assert.Equal(7, TypeStaticFields.s9.x1);
    }

    [UnitTest]
    public void valuetypesize_16()
    {
        Assert.Equal(8, TypeStaticFields.s16.x1);
    }
}

public class TypeThreadStaticFields
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

internal class TestLdsfldThreadstatic
{
    [UnitTest]
    public void Init()
    {
        TypeThreadStaticFields.Init();
    }

    [UnitTest]
    public void byte_1()
    {
        Assert.Equal(1, TypeThreadStaticFields.x1);
    }

    [UnitTest]
    public void sbyte_1()
    {
        Assert.Equal(2, TypeThreadStaticFields.x2);
    }

    [UnitTest]
    public void bool_1()
    {
        Assert.True(TypeThreadStaticFields.x3);
    }

    [UnitTest]
    public void short_1()
    {
        Assert.Equal(5, TypeThreadStaticFields.x5);
    }

    [UnitTest]
    public void ushort_1()
    {
        Assert.Equal(6, TypeThreadStaticFields.x6);
    }

    [UnitTest]
    public void int_1()
    {
        Assert.Equal(7, TypeThreadStaticFields.x7);
    }

    [UnitTest]
    public void uint_1()
    {
        Assert.Equal(8, TypeThreadStaticFields.x8);
    }

    [UnitTest]
    public void long_1()
    {
        Assert.Equal(9, TypeThreadStaticFields.x9);
    }

    [UnitTest]
    public void ulong_1()
    {
        Assert.Equal(10, TypeThreadStaticFields.x10);
    }

    [UnitTest]
    public void float_1()
    {
        Assert.Equal(1f, TypeThreadStaticFields.y1);
    }

    [UnitTest]
    public void double_1()
    {
        Assert.Equal(3, TypeThreadStaticFields.y3);
    }

    [UnitTest]
    public void str_1()
    {
        Assert.Equal("a", TypeThreadStaticFields.y4);
    }

    [UnitTest]
    public void enum_byte_1()
    {
        Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields.e1);
    }

    [UnitTest]
    public void enum_sbyte_1()
    {
        Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields.e2);
    }

    [UnitTest]
    public void enum_short_1()
    {
        Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields.e3);
    }

    [UnitTest]
    public void enum_ushort_1()
    {
        Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields.e4);
    }

    [UnitTest]
    public void enum_int_1()
    {
        Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields.e5);
    }

    [UnitTest]
    public void enum_uint_1()
    {
        Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields.e6);
    }

    [UnitTest]
    public void enum_long_1()
    {
        Assert.Equal(AOT_Enum_long.B, TypeThreadStaticFields.e7);
    }

    [UnitTest]
    public void enum_ulong_1()
    {
        Assert.Equal(AOT_Enum_ulong.A, TypeThreadStaticFields.e8);
    }

    [UnitTest]
    public void valuetypesize_1()
    {
        Assert.Equal(1, TypeThreadStaticFields.s1.x1);
    }

    [UnitTest]
    public void valuetypesize_2()
    {
        Assert.Equal(2, TypeThreadStaticFields.s2.x1);
    }

    [UnitTest]
    public void valuetypesize_3()
    {
        Assert.Equal(3, TypeThreadStaticFields.s3.x1);
    }

    [UnitTest]
    public void valuetypesize_4()
    {
        Assert.Equal(4, TypeThreadStaticFields.s4.x1);
    }

    [UnitTest]
    public void valuetypesize_5()
    {
        Assert.Equal(5, TypeThreadStaticFields.s5.x1);
    }

    [UnitTest]
    public void valuetypesize_8()
    {
        Assert.Equal(6, TypeThreadStaticFields.s8.x1);
    }

    [UnitTest]
    public void valuetypesize_9()
    {
        Assert.Equal(7, TypeThreadStaticFields.s9.x1);
    }

    [UnitTest]
    public void valuetypesize_16()
    {
        Assert.Equal(8, TypeThreadStaticFields.s16.x1);
    }
}
