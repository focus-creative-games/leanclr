using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public struct GenericValueType<T>
{
    public T Value { get; set; }
}

internal class TestNullable
{
    [UnitTest]
    public void int_1()
    {
        var a = (int?)1;
        Assert.True(a == 1);
    }

    [UnitTest]
    public void int_GetValueOrDefault1()
    {
        int? a = 1;
        Assert.Equal(1, a.GetValueOrDefault());
        a = null;
        Assert.Equal(0, a.GetValueOrDefault());
        Assert.Equal(2, a.GetValueOrDefault(2));
    }


    [UnitTest]
    public void char_1()
    {
        var a = (char?)'.';
        Assert.True(a == '.');
    }

    [UnitTest]
    public void generic_1()
    {
        GenericValueType<byte>? s = default;
        Assert.False(s.HasValue);
        byte? a = s?.Value;
        Assert.False(a.HasValue);
        GenericValueType<byte> b = s ?? new GenericValueType<byte>() { Value = 1 };
        Assert.Equal(1, b.Value);
    }

    [UnitTest]
    public void generic_2()
    {
        GenericValueType<byte>? s = new GenericValueType<byte>() { Value = 1 };
        Assert.True(s.HasValue);
        byte? a = s?.Value;
        Assert.True(a.HasValue);
        Assert.Equal(1, a.Value);
        GenericValueType<byte> b = s ?? new GenericValueType<byte>() { Value = 2 };
        Assert.Equal(1, b.Value);
    }

    private static bool HasValue<T>(T? x) where T : struct
    {
        return x.HasValue;
    }

    private static T GetValue<T>(T? x) where T : struct
    {
        return x.Value;
    }

    [UnitTest]
    public void coherence_bool()
    {
        bool? x = null;
        Assert.False(HasValue(x));

        bool? y = true;
        Assert.True(HasValue(y));
        Assert.True(GetValue(y));

        bool? z = false;
        Assert.True(HasValue(z));
        Assert.False(GetValue(z));
    }

    [UnitTest]
    public void coherence_byte()
    {
        byte? x = null;
        Assert.False(HasValue(x));

        byte? y = 3;
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y));
    }

    [UnitTest]
    public void coherence_short()
    {
        short? x = null;
        Assert.False(HasValue(x));

        short? y = 3;
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y));
    }

    [UnitTest]
    public void coherence_int()
    {
        int? x = null;
        Assert.False(HasValue(x));

        int? y = 3;
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y));
    }

    [UnitTest]
    public void coherence_long()
    {
        long? x = null;
        Assert.False(HasValue(x));

        long? y = 3;
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y));
    }

    [UnitTest]
    public void coherence_byte_enum()
    {
        AOT_Enum_byte? x = null;
        Assert.False(HasValue(x));

        AOT_Enum_byte? y = AOT_Enum_byte.A;
        Assert.True(HasValue(y));
        Assert.Equal(AOT_Enum_byte.A, GetValue(y));
    }

    [UnitTest]
    public void coherence_short_enum()
    {
        AOT_Enum_short? x = null;
        Assert.False(HasValue(x));

        AOT_Enum_short? y = AOT_Enum_short.A;
        Assert.True(HasValue(y));
        Assert.Equal(AOT_Enum_short.A, GetValue(y));
    }

    [UnitTest]
    public void coherence_int_enum()
    {
        AOT_Enum_int? x = null;
        Assert.False(HasValue(x));

        AOT_Enum_int? y = AOT_Enum_int.A;
        Assert.True(HasValue(y));
        Assert.Equal(AOT_Enum_int.A, GetValue(y));
    }

    [UnitTest]
    public void coherence_long_enum()
    {
        AOT_Enum_long? x = null;
        Assert.False(HasValue(x));

        AOT_Enum_long? y = AOT_Enum_long.A;
        Assert.True(HasValue(y));
        Assert.Equal(AOT_Enum_long.A, GetValue(y));
    }

    [UnitTest]
    public void coherence_ValueTypeSize1()
    {
        ValueTypeSize1? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize1? y = new ValueTypeSize1 { x1 = 3 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize2()
    {
        ValueTypeSize2? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize2? y = new ValueTypeSize2 { x1 = 3, x2 = 4 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize3()
    {
        ValueTypeSize3? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize3? y = new ValueTypeSize3 { x1 = 3, x2 = 4, x3 = 5 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize4()
    {
        ValueTypeSize4? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize4 y = new ValueTypeSize4 { x1 = 3 };
        Assert.True(HasValue<ValueTypeSize4>(y));
        Assert.Equal(3, GetValue<ValueTypeSize4>(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize5()
    {
        ValueTypeSize5? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize5? y = new ValueTypeSize5 { x1 = 3, x2 = 4, x3 = 5, x4 = 6, x5 = 7 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize8()
    {
        ValueTypeSize8? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize8? y = new ValueTypeSize8 { x1 = 3 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize9()
    {
        ValueTypeSize9? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize9? y = new ValueTypeSize9 { x1 = 3, x2 = 4, x3 = 5, x4 = 6, x5 = 7, x6 = 8, x7 = 9, x8 = 10, x9 = 11 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize16()
    {
        ValueTypeSize16? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize16? y = new ValueTypeSize16 { x1 = 3, x2 = 4 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTypeSize20()
    {
        ValueTypeSize20? x = null;
        Assert.False(HasValue(x));

        ValueTypeSize20? y = new ValueTypeSize20 { x1 = 3, x2 = 4, x3 = 5, x4 = 6, x5 = 7 };
        Assert.True(HasValue(y));
        Assert.Equal(3, GetValue(y).x1);
    }

    [UnitTest]
    public void coherence_ValueTuple3()
    {
        ValueTuple<int, int, int>? x = null;
        Assert.False(HasValue(x));

        ValueTuple<int, int, int>? y = new ValueTuple<int, int, int>(10, 20, 30);
        Assert.True(HasValue(y));
        ValueTuple<int, int, int> z = GetValue(y);
        Assert.Equal(10, z.Item1);
        Assert.Equal(20, z.Item2);
        Assert.Equal(30, z.Item3);
    }
}