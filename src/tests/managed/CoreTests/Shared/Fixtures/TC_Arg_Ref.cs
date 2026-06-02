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
    public static class TC_Arg_Ref
    {
        // ref

        public static int Arg_bool(ref bool x)
        {
            return x ? 1 : 0;
        }

        public static int Arg_byte(ref byte x)
        {
            return x;
        }

        public static int Arg_sbyte(ref sbyte x)
        {
            return x;
        }

        public static int Arg_ushort(ref ushort x)
        {
            return x;
        }

        public static int Arg_short(ref short x)
        {
            return x;
        }

        public static int Arg_char(ref char x)
        {
            return x;
        }

        public static uint Arg_uint(ref uint x)
        {
            return x;
        }

        public static int Arg_int(ref int x)
        {
            return x;
        }

        public static ulong Arg_ulong(ref ulong x)
        {
            return x;
        }

        public static long Arg_long(ref long x)
        {
            return x;
        }

        public static object Arg_object(ref object x)
        {
            return x;
        }


        public static float Arg_float(ref float x)
        {
            return x;
        }

        public static double Arg_double(ref double x)
        {
            return x;
        }
        // enum


        public static int Arg_enum_byte(ref AOT_Enum_byte x)
        {
            return (int)x;
        }

        public static int Arg_enum_sbyte(ref AOT_Enum_sbyte x)
        {
            return (int)x;
        }

        public static int Arg_enum_ushort(ref AOT_Enum_ushort x)
        {
            return (int)x;
        }

        public static int Arg_enum_short(ref AOT_Enum_short x)
        {
            return (int)x;
        }

        public static uint Arg_enum_uint(ref AOT_Enum_uint x)
        {
            return (uint)x;
        }

        public static int Arg_enum_int(ref AOT_Enum_int x)
        {
            return (int)x;
        }

        public static ulong Arg_enum_ulong(ref AOT_Enum_ulong x)
        {
            return (ulong)x;
        }

        public static long Arg_enum_long(ref AOT_Enum_long x)
        {
            return (long)x;
        }

        public static ValueTypeSize0 Arg_ValueTypeSize0(ref ValueTypeSize0 x)
        {
            return x;
        }

        public static ValueTypeSize1 Arg_ValueTypeSize1(ref ValueTypeSize1 x)
        {
            return x;
        }

        public static ValueTypeSize2 Arg_ValueTypeSize2(ref ValueTypeSize2 x)
        {
            return x;
        }

        public static ValueTypeSize3 Arg_ValueTypeSize3(ref ValueTypeSize3 x)
        {
            return x;
        }

        public static ValueTypeSize4 Arg_ValueTypeSize4(ref ValueTypeSize4 x)
        {
            return x;
        }

        public static ValueTypeSize5 Arg_ValueTypeSize5(ref ValueTypeSize5 x)
        {
            return x;
        }

        public static ValueTypeSize8 Arg_ValueTypeSize8(ref ValueTypeSize8 x)
        {
            return x;
        }

        public static ValueTypeSize9 Arg_ValueTypeSize9(ref ValueTypeSize9 x)
        {
            return x;
        }

        public static ValueTypeSize16 Arg_ValueTypeSize16(ref ValueTypeSize16 x)
        {
            return x;
        }

        public static ValueTypeSize20 Arg_ValueTypeSize20(ref ValueTypeSize20 x)
        {
            return x;
        }



        public static NotHFA Arg_NotHFA(ref NotHFA x)
        {
            return x;
        }

        public static EmbedHFA Arg_EmbedHFA(ref EmbedHFA x)
        {
            return x;
        }

        public static EmbedHFA2 Arg_EmbedHFA2(ref EmbedHFA2 x)
        {
            return x;
        }

        // HFA

        public static Vector2f Arg_Vector2f(ref Vector2f x)
        {
            return x;
        }

        public static Vector3f Arg_Vector3f(ref Vector3f x)
        {
            return x;
        }

        public static Vector4f Arg_Vector4f(ref Vector4f x)
        {
            return x;
        }

        public static Vector2d Arg_Vector2d(ref Vector2d x)
        {
            return x;
        }

        public static Vector3d Arg_Vector3d(ref Vector3d x)
        {
            return x;
        }

        public static Vector4d Arg_Vector4d(ref Vector4d x)
        {
            return x;
        }
    }
}
