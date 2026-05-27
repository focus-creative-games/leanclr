
public class InterpTypeFields
{
    public byte x1 = 1;
    public sbyte x2 = 2;
    public bool x3 = true;
    public byte x4 = 4;
    public short x5 = 5;
    public ushort x6 = 6;
    public int x7 = 7;
    public uint x8 = 8;
    public long x9 = 9;
    public ulong x10 = 10;
    public float y1 = 1f;
    public float y2 = 2f;
    public double y3 = 3.0;
    public object y4 = "a";
    public ValueTypeSize1 s1 = new ValueTypeSize1 { x1 = 1 };
    public ValueTypeSize2 s2 = new ValueTypeSize2 { x1 = 2 };
    public ValueTypeSize3 s3 = new ValueTypeSize3 { x1 = 3 };
    public ValueTypeSize4 s4 = new ValueTypeSize4 { x1 = 4 };
    public ValueTypeSize5 s5 = new ValueTypeSize5 { x1 = 5 };
    public ValueTypeSize8 s8 = new ValueTypeSize8 { x1 = 6 };
    public ValueTypeSize9 s9 = new ValueTypeSize9 { x1 = 7 };
    public ValueTypeSize16 s16 = new ValueTypeSize16 { x1 = 8 };

    public AOT_Enum_byte e1 = AOT_Enum_byte.A;
    public AOT_Enum_sbyte e2 = AOT_Enum_sbyte.B;
    public AOT_Enum_short e3 = AOT_Enum_short.A;
    public AOT_Enum_ushort e4 = AOT_Enum_ushort.A;
    public AOT_Enum_int e5 = AOT_Enum_int.B;
    public AOT_Enum_uint e6 = AOT_Enum_uint.A;
    public AOT_Enum_long e7 = AOT_Enum_long.B;
    public AOT_Enum_ulong e8 = AOT_Enum_ulong.A;
}

public class TestLdfld
{
    [UnitTest]
    public void byte_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(1, t.x1);
    }

    [UnitTest]
    public void sbyte_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(2, t.x2);
    }

    [UnitTest]
    public void bool_1()
    {
        var t = new InterpTypeFields();
        Assert.True(t.x3);
    }

    [UnitTest]
    public void short_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(5, t.x5);
    }

    [UnitTest]
    public void ushort_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(6, t.x6);
    }

    [UnitTest]
    public void int_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(7, t.x7);
    }

    [UnitTest]
    public void uint_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(8, t.x8);
    }

    [UnitTest]
    public void long_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(9, t.x9);
    }

    [UnitTest]
    public void ulong_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(10, t.x10);
    }

    [UnitTest]
    public void float_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(1f, t.y1);
    }

    [UnitTest]
    public void double_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(3, t.y3);
    }

    [UnitTest]
    public void str_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal("a", t.y4);
    }

    [UnitTest]
    public void enum_byte_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_byte.A, t.e1);
    }

    [UnitTest]
    public void enum_sbyte_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_sbyte.B, t.e2);
    }

    [UnitTest]
    public void enum_short_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_short.A, t.e3);
    }

    [UnitTest]
    public void enum_ushort_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_ushort.A, t.e4);
    }

    [UnitTest]
    public void enum_int_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_int.B, t.e5);
    }

    [UnitTest]
    public void enum_uint_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_uint.A, t.e6);
    }

    [UnitTest]
    public void enum_long_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_long.B, t.e7);
    }

    [UnitTest]
    public void enum_ulong_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(AOT_Enum_ulong.A, t.e8);
    }

    [UnitTest]
    public void valuetypesize_1()
    {
        var t = new InterpTypeFields();
        Assert.Equal(1, t.s1.x1);
    }

    [UnitTest]
    public void valuetypesize_2()
    {
        var t = new InterpTypeFields();
        Assert.Equal(2, t.s2.x1);
    }

    [UnitTest]
    public void valuetypesize_3()
    {
        var t = new InterpTypeFields();
        Assert.Equal(3, t.s3.x1);
    }

    [UnitTest]
    public void valuetypesize_4()
    {
        var t = new InterpTypeFields();
        Assert.Equal(4, t.s4.x1);
    }

    [UnitTest]
    public void valuetypesize_5()
    {
        var t = new InterpTypeFields();
        Assert.Equal(5, t.s5.x1);
    }

    [UnitTest]
    public void valuetypesize_8()
    {
        var t = new InterpTypeFields();
        Assert.Equal(6, t.s8.x1);
    }

    [UnitTest]
    public void valuetypesize_9()
    {
        var t = new InterpTypeFields();
        Assert.Equal(7, t.s9.x1);
    }

    [UnitTest]
    public void valuetypesize_16()
    {
        var t = new InterpTypeFields();
        Assert.Equal(8, t.s16.x1);
    }
}

