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
    public static class TC_Arg_In
    {
        public static int Arg_bool(in bool x)
        {
            return x ? 1 : 0;
        }

        public static int Arg_byte(in byte x)
        {
            return x;
        }

        public static int Arg_sbyte(in sbyte x)
        {
            return x;
        }

        public static int Arg_ushort(in ushort x)
        {
            return x;
        }

        public static int Arg_short(in short x)
        {
            return x;
        }

        public static int Arg_char(in char x)
        {
            return x;
        }

        public static uint Arg_uint(in uint x)
        {
            return x;
        }

        public static int Arg_int(in int x)
        {
            return x;
        }

        public static ulong Arg_ulong(in ulong x)
        {
            return x;
        }

        public static long Arg_long(in long x)
        {
            return x;
        }

        public static object Arg_object(in object x)
        {
            return x;
        }


        public static float Arg_float(in float x)
        {
            return x;
        }

        public static double Arg_double(in double x)
        {
            return x;
        }
        // enum


        public static int Arg_enum_byte(in AOT_Enum_byte x)
        {
            return (int)x;
        }

        public static int Arg_enum_sbyte(in AOT_Enum_sbyte x)
        {
            return (int)x;
        }

        public static int Arg_enum_ushort(in AOT_Enum_ushort x)
        {
            return (int)x;
        }

        public static int Arg_enum_short(in AOT_Enum_short x)
        {
            return (int)x;
        }

        public static uint Arg_enum_uint(in AOT_Enum_uint x)
        {
            return (uint)x;
        }

        public static int Arg_enum_int(in AOT_Enum_int x)
        {
            return (int)x;
        }

        public static ulong Arg_enum_ulong(in AOT_Enum_ulong x)
        {
            return (ulong)x;
        }

        public static long Arg_enum_long(in AOT_Enum_long x)
        {
            return (long)x;
        }

        public static ValueTypeSize0 Arg_ValueTypeSize0(in ValueTypeSize0 x)
        {
            return x;
        }

        public static ValueTypeSize1 Arg_ValueTypeSize1(in ValueTypeSize1 x)
        {
            return x;
        }

        public static ValueTypeSize2 Arg_ValueTypeSize2(in ValueTypeSize2 x)
        {
            return x;
        }

        public static ValueTypeSize3 Arg_ValueTypeSize3(in ValueTypeSize3 x)
        {
            return x;
        }

        public static ValueTypeSize4 Arg_ValueTypeSize4(in ValueTypeSize4 x)
        {
            return x;
        }

        public static ValueTypeSize5 Arg_ValueTypeSize5(in ValueTypeSize5 x)
        {
            return x;
        }

        public static ValueTypeSize8 Arg_ValueTypeSize8(in ValueTypeSize8 x)
        {
            return x;
        }

        public static ValueTypeSize9 Arg_ValueTypeSize9(in ValueTypeSize9 x)
        {
            return x;
        }

        public static ValueTypeSize16 Arg_ValueTypeSize16(in ValueTypeSize16 x)
        {
            return x;
        }

        public static ValueTypeSize20 Arg_ValueTypeSize20(in ValueTypeSize20 x)
        {
            return x;
        }



        public static NotHFA Arg_NotHFA(in NotHFA x)
        {
            return x;
        }

        public static EmbedHFA Arg_EmbedHFA(in EmbedHFA x)
        {
            return x;
        }

        public static EmbedHFA2 Arg_EmbedHFA2(in EmbedHFA2 x)
        {
            return x;
        }

        // HFA

        public static Vector2f Arg_Vector2f(in Vector2f x)
        {
            return x;
        }

        public static Vector3f Arg_Vector3f(in Vector3f x)
        {
            return x;
        }

        public static Vector4f Arg_Vector4f(in Vector4f x)
        {
            return x;
        }


        public delegate Vector2d PassVec2d(in Vector2d x);
        public delegate Vector3d PassVec3d(in Vector3d x);
        public delegate Vector4d PassVec4d(in Vector4d x);

        public static Vector2d Arg_Vector2d(in Vector2d x)
        {
            return default(PassVec2d)(in x);
        }

        public static Vector3d Arg_Vector3d(in Vector3d x)
        {
            return default(PassVec3d)(in x);
        }

        public static Vector4d Arg_Vector4d(in Vector4d x)
        {
            return default(PassVec4d)(in x);
        }
    }
}
