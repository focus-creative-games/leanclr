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
    public static class TC_Ret
    {
        public delegate sbyte F1();
        public delegate ushort F2();
        public delegate short F3();
        public delegate uint F4();
        public delegate ulong F5();
        public delegate float F6();
        public delegate double F7();
        public delegate long F8();

        public delegate ValueTypeSize1 S1();
        public delegate ValueTypeSize2 S2();
        public delegate ValueTypeSize3 S3();
        public delegate ValueTypeSize4 S4();
        public delegate ValueTypeSize5 S5();
        public delegate ValueTypeSize8 S8();
        public delegate ValueTypeSize9 S9();
        public delegate ValueTypeSize20 S20();

        public delegate NotHFA H1();
        public delegate EmbedHFA H2();
        public delegate EmbedHFA2 H3();

        public delegate Vector2f V2();
        public delegate Vector3f V3();
        public delegate Vector4f V4();
        public delegate Vector2d D2();
        public delegate Vector3d D3();
        public delegate Vector4d D4();

        public delegate void DelRun(ref int a, int b);
        public delegate int DelShow(ref int a, int b);
        public delegate sbyte DelFoo(ref int a, int b);

        public delegate sbyte Foo(int a);
        public delegate sbyte Foo2(string a, int b);


        public interface IFoo
        {
            sbyte F1(int b);
        }


        public static bool Ret_bool_true()
        {
            return true;
        }

        public static byte Ret_byte_1()
        {
            return 1;
        }

        public static sbyte Ret_sbyte_minus1()
        {
            return -1;
        }

        public static ushort Ret_ushort()
        {
            return 0x1122;
        }

        public static short Ret_short()
        {
            return -0x1122;
        }

        public static char Ret_char()
        {
            return 'a';
        }

        public static uint Ret_uint()
        {
            return 0x11223344;
        }

        public static int Ret_int()
        {
            return -0x11223344;
        }

        public static ulong Ret_ulong()
        {
            return 0x1122334455667788L;
        }

        public static long Ret_long()
        {
            return -0x1122334455667788L;
        }

        public static float Ret_float()
        {
            return 1234f;
        }

        public static double Ret_double()
        {
            return 1234567890.0;
        }

        public static object Ret_null()
        {
            return null;
        }

        public static AOT_Enum_byte Ret_enum_byte()
        {
            return AOT_Enum_byte.A;
        }

        public static AOT_Enum_sbyte Ret_enum_sbyte()
        {
            return AOT_Enum_sbyte.A;
        }

        public static AOT_Enum_short Ret_enum_short()
        {
            return AOT_Enum_short.A;
        }

        public static AOT_Enum_ushort Ret_enum_ushort()
        {
            return AOT_Enum_ushort.A;
        }

        public static AOT_Enum_int Ret_enum_int()
        {
            return AOT_Enum_int.A;
        }

        public static AOT_Enum_uint Ret_enum_uint()
        {
            return AOT_Enum_uint.A;
        }

        public static AOT_Enum_long Ret_enum_long()
        {
            return AOT_Enum_long.A;
        }

        public static AOT_Enum_ulong Ret_enum_ulong()
        {
            return AOT_Enum_ulong.A;
        }


        // normal struct
        public static unsafe ValueTypeSize0 Ret_ValueTypeSize0()
        {
            ValueTypeSize0 v = default;
            *(byte*)&v = 0;
            return v;
        }

        public static ValueTypeSize1 Ret_ValueTypeSize1()
        {
            ValueTypeSize1 v = default;
            v.x1 = 1;
            return v;
        }

        public static ValueTypeSize2 Ret_ValueTypeSize2()
        {
            ValueTypeSize2 v = default;
            v.x1 = 1;
            v.x2 = 2;
            return v;
        }

        public static ValueTypeSize3 Ret_ValueTypeSize3()
        {
            ValueTypeSize3 v = default;
            v.x1 = 1;
            v.x2 = 2;
            v.x3 = 3;
            return v;
        }

        public static ValueTypeSize4 Ret_ValueTypeSize4()
        {
            ValueTypeSize4 v = default;
            v.x1 = 1;
            return v;
        }

        public static ValueTypeSize5 Ret_ValueTypeSize5()
        {
            ValueTypeSize5 v = default;
            v.x1 = 1;
            v.x2 = 2;
            v.x3 = 3;
            v.x4 = 4;
            v.x5 = 5;
            return v;
        }

        public static ValueTypeSize8 Ret_ValueTypeSize8()
        {
            ValueTypeSize8 v = default;
            v.x1 = 1;
            return v;
        }

        public static ValueTypeSize9 Ret_ValueTypeSize9()
        {
            ValueTypeSize9 v = default;
            v.x1 = 1;
            v.x2 = 2;
            v.x3 = 3;
            v.x4 = 4;
            v.x5 = 5;
            v.x6 = 6;
            v.x7 = 7;
            v.x8 = 8;
            v.x9 = 9;
            return v;
        }

        public static ValueTypeSize16 Ret_ValueTypeSize16()
        {
            ValueTypeSize16 v = default;
            v.x1 = 1;
            v.x2 = 2;
            return v;
        }

        public static ValueTypeSize20 Ret_ValueTypeSize20()
        {
            ValueTypeSize20 v = default;
            v.x1 = 1;
            v.x2 = 2;
            v.x3 = 3;
            v.x4 = 4;
            v.x5 = 5;
            return v;
        }

        public static NotHFA Ret_NotHFA()
        {
            NotHFA x = default;
            x.x = 1f;
            x.y = 2f;
            x.z = 3;
            return x;
        }

        public static EmbedHFA Ret_EmbedHFA()
        {
            EmbedHFA x = default;
            x.x.x = 1f;
            x.x.y = 2f;
            x.y = 3f;
            return x;
        }

        public static EmbedHFA2 Ret_EmbedHFA2()
        {
            EmbedHFA2 x = default;
            x.x.x = 1f;
            x.x.y = 2f;
            x.y.x = 3f;
            x.y.y = 4f;
            return x;
        }

        // HFA

        public static Vector2f Ret_Vector2f()
        {
            Vector2f v = default;
            v.x = 1f;
            v.y = 2f;
            return v;
        }

        public static Vector3f Ret_Vector3f()
        {
            Vector3f v = default;
            v.x = 1f;
            v.y = 2f;
            v.z = 3f;
            return v;
        }

        public static Vector4f Ret_Vector4f()
        {
            Vector4f v = default;
            v.x = 1f;
            v.y = 2f;
            v.z = 3f;
            v.w = 4f;
            return v;
        }

        public static Vector2d Ret_Vector2d()
        {
            Vector2d v = default;
            v.x = 1.0;
            v.y = 2.0;
            return v;
        }

        public static Vector3d Ret_Vector3d()
        {
            Vector3d v = default;
            v.x = 1.0;
            v.y = 2.0;
            v.z = 3.0;
            return v;
        }

        public static Vector4d Ret_Vector4d()
        {
            Vector4d v = default;
            v.x = 1.0;
            v.y = 2.0;
            v.z = 3.0;
            v.w = 4.0;
            return v;
        }
    }
}
