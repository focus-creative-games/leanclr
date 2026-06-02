using AOTDefs;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace AOTDefs
{
    /// <summary>
    /// TC 是 TestClass的缩写
    /// </summary>
    public static class TC_Arg
    {

        public static int Arg_bool(bool x)
        {
            return default(Func<bool, int>)(x);
        }

        public static int Arg_byte(byte x)
        {
            return default(Func<byte, int>)(x);
        }

        public static int Arg_sbyte(sbyte x)
        {
            return default(Func<sbyte, int>)(x);
        }

        public static int Arg_ushort(ushort x)
        {
            return default(Func<ushort, int>)(x);
        }

        public static int Arg_short(short x)
        {
            return default(Func<short, int>)(x);
        }

        public static int Arg_char(char x)
        {
            return default(Func<char, int>)(x);
        }

        public static uint Arg_uint(uint x)
        {
            return default(Func<uint, uint>)(x);
        }

        public static int Arg_int(int x)
        {
            return default(Func<int, int>)(x);
        }

        public static ulong Arg_ulong(ulong x)
        {
            return default(Func<ulong, ulong>)(x);
        }

        public static long Arg_long(long x)
        {
            return default(Func<long, long>)(x);
        }

        public static float Arg_float(float x)
        {
            return default(Func<float, float>)(x);
        }

        public static double Arg_double(double x)
        {
            return default(Func<double, double>)(x);
        }

        public static object Arg_null(object x)
        {
            return default(Func<object, object>)(x);
        }

        public static int Arg_enum_byte(AOT_Enum_byte x)
        {
            return default(Func<AOT_Enum_byte, int>)(x);
        }

        public static int Arg_enum_sbyte(AOT_Enum_sbyte x)
        {
            return default(Func<AOT_Enum_sbyte, int>)(x);
        }

        public static int Arg_enum_short(AOT_Enum_short x)
        {
            return default(Func<AOT_Enum_short, int>)(x);
        }

        public static int Arg_enum_ushort(AOT_Enum_ushort x)
        {
            return default(Func<AOT_Enum_ushort, int>)(x);
        }

        public static int Arg_enum_int(AOT_Enum_int x)
        {
            return default(Func<AOT_Enum_int, int>)(x);
        }

        public static uint Arg_enum_uint(AOT_Enum_uint x)
        {
            return default(Func<AOT_Enum_uint, uint>)(x);
        }

        public static long Arg_enum_long(AOT_Enum_long x)
        {
            return default(Func<AOT_Enum_long, long>)(x);
        }

        public static ulong Arg_enum_ulong(AOT_Enum_ulong x)
        {
            return default(Func<AOT_Enum_ulong, ulong>)(x);
        }

        // normal struct
        public static ValueTypeSize0 Arg_ValueTypeSize0(ValueTypeSize0 x)
        {
            return default(Func<ValueTypeSize0, ValueTypeSize0>)(x);
        }

        public static ValueTypeSize1 Arg_ValueTypeSize1(ValueTypeSize1 x)
        {
            return default(Func<ValueTypeSize1, ValueTypeSize1>)(x);
        }

        public static ValueTypeSize2 Arg_ValueTypeSize2(ValueTypeSize2 x)
        {
            return default(Func<ValueTypeSize2, ValueTypeSize2>)(x);
        }

        public static ValueTypeSize3 Arg_ValueTypeSize3(ValueTypeSize3 x)
        {
            return default(Func<ValueTypeSize3, ValueTypeSize3>)(x);
        }

        public static ValueTypeSize4 Arg_ValueTypeSize4(ValueTypeSize4 x)
        {
            return default(Func<ValueTypeSize4, ValueTypeSize4>)(x);
        }

        public static ValueTypeSize5 Arg_ValueTypeSize5(ValueTypeSize5 x)
        {
            return default(Func<ValueTypeSize5, ValueTypeSize5>)(x);
        }

        public static ValueTypeSize8 Arg_ValueTypeSize8(ValueTypeSize8 x)
        {
            return default(Func<ValueTypeSize8, ValueTypeSize8>)(x);
        }

        public static ValueTypeSize9 Arg_ValueTypeSize9(ValueTypeSize9 x)
        {
            return default(Func<ValueTypeSize9, ValueTypeSize9>)(x);
        }

        public static ValueTypeSize16 Arg_ValueTypeSize16(ValueTypeSize16 x)
        {
            return default(Func<ValueTypeSize16, ValueTypeSize16>)(x);
        }

        public static ValueTypeSize20 Arg_ValueTypeSize20(ValueTypeSize20 x)
        {
            return default(Func<ValueTypeSize20, ValueTypeSize20>)(x);
        }

        public static NotHFA Arg_NotHFA(NotHFA x)
        {
            return default(Func<NotHFA, NotHFA>)(x);
        }

        public static EmbedHFA Arg_EmbedHFA(EmbedHFA x)
        {
            return default(Func<EmbedHFA, EmbedHFA>)(x);
        }

        public static EmbedHFA2 Arg_EmbedHFA2(EmbedHFA2 x)
        {
            return default(Func<EmbedHFA2, EmbedHFA2>)(x);
        }

        // HFA

        public static Vector2f Arg_Vector2f(Vector2f x)
        {
            return default(Func<Vector2f, Vector2f>)(x);
        }

        public static Vector3f Arg_Vector3f(Vector3f x)
        {
            return default(Func<Vector3f, Vector3f>)(x);
        }

        public static Vector4f Arg_Vector4f(Vector4f x)
        {
            return default(Func<Vector4f, Vector4f>)(x);
        }

        public static Vector2d Arg_Vector2d(Vector2d x)
        {
            return default(Func<Vector2d, Vector2d>)(x);
        }

        public static Vector3d Arg_Vector3d(Vector3d x)
        {
            return default(Func<Vector3d, Vector3d>)(x);
        }

        public static Vector4d Arg_Vector4d(Vector4d x)
        {
            return default(Func<Vector4d, Vector4d>)(x);
        }

    }
}
