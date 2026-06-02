using AOTDefs;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace AOTDefs
{

    public static class TestAOTReturn
    {
        public static sbyte Return_sbyte_minus1()
        {
            return -1;
        }
        public static sbyte Return_sbyte_1()
        {
            return 1;
        }

        public static byte Return_byte_1()
        {
            return 1;
        }

        public static short Return_short_minus1()
        {
            return -1;
        }

        public static short Return_short_1()
        {
            return 1;
        }

        public static ushort Return_ushort_1()
        {
            return 1;
        }

        public static int Return_int_minus1()
        {
            return -1;
        }

        public static int Return_int_1()
        {
            return 1;
        }

        public static uint Return_uint_1()
        {
            return 1;
        }

        public static long Return_long_minus_0x1122334455667788()
        {
            return -0x1122334455667788L;
        }

        public static long Return_long_0x1122334455667788()
        {
            return 0x1122334455667788L;
        }

        public static ulong Return_ulong_0x1122334455667788()
        {
            return 0x1122334455667788UL;
        }


        public static float Return_float_1()
        {
            return 1f;
        }
        public static double Return_double_1()
        {
            return 1.0;
        }

        public static AOT_Enum_sbyte Return_sbyte_enum_A()
        {
            return AOT_Enum_sbyte.A;
        }

        public static AOT_Enum_sbyte Return_sbyte_enum_B()
        {
            return AOT_Enum_sbyte.B;
        }

        public static AOT_Enum_byte Return_byte_enum_A()
        {
            return AOT_Enum_byte.A;
        }

        public static AOT_Enum_short Return_short_enum_A()
        {
            return AOT_Enum_short.A;
        }

        public static AOT_Enum_short Return_short_enum_B()
        {
            return AOT_Enum_short.B;
        }

        public static AOT_Enum_ushort Return_ushort_enum_A()
        {
            return AOT_Enum_ushort.A;
        }

        public static AOT_Enum_int Return_int_enum_A()
        {
            return AOT_Enum_int.A;
        }

        public static AOT_Enum_uint Return_uint_enum_A()
        {
            return AOT_Enum_uint.A;
        }

        public static AOT_Enum_int Return_int_enum_B()
        {
            return AOT_Enum_int.B;
        }

        public static AOT_Enum_long Return_long_enum_A()
        {
            return AOT_Enum_long.A;
        }

        public static AOT_Enum_long Return_long_enum_B()
        {
            return AOT_Enum_long.B;
        }

        public static AOT_Enum_ulong Return_ulong_enum_A()
        {
            return AOT_Enum_ulong.A;
        }

        public static Vector2 Return_Vector2_1_2()
        {
            return new Vector2(1, 2);
        }

        public static Vector3 Return_Vector3_1_2_3()
        {
            return new Vector3(1, 2, 3);
        }

        public static Vector4 Return_Vector4_1_2_3_4()
        {
            return new Vector4(1, 2, 3, 4);
        }


        public static ValueTypeSize1 Return_ValueTypeSize1()
        {
            return new ValueTypeSize1 { x1 = 1 };
        }

        public static ValueTypeSize2 Return_ValueTypeSize2()
        {
            return new ValueTypeSize2 { x1 = 1, x2 = 2 };
        }

        public static ValueTypeSize3 Return_ValueTypeSize3()
        {
            return new ValueTypeSize3 { x1 = 1, x2 = 2, x3 = 3 };
        }

        public static ValueTypeSize4 Return_ValueTypeSize4()
        {
            return new ValueTypeSize4 { x1 = 1 };
        }

        public static ValueTypeSize5 Return_ValueTypeSize5()
        {
            return new ValueTypeSize5 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
        }

        public static ValueTypeSize8 Return_ValueTypeSize8()
        {
            return new ValueTypeSize8 { x1 = 1 };
        }

        public static ValueTypeSize9 Return_ValueTypeSize9()
        {
            return new ValueTypeSize9
            {
                x1 = 1,
                x2 = 2,
                x3 = 3,
                x4 = 4,
                x5 = 5,
                x6 = 6,
                x7 = 7,
                x8 = 8,
                x9 = 9,
            };
        }


        public static ValueTypeSize16 Return_ValueTypeSize16()
        {
            return new ValueTypeSize16
            {
                x1 = 1,
                x2 = 2,
            };
        }

        public static ValueTypeSize20 Return_ValueTypeSize20()
        {
            return new ValueTypeSize20
            {
                x1 = 1,
                x5 = 2,
            };
        }

        public static string Return_string_abc()
        {
            return "abc";
        }

        public static NotHFA NotHFA_Return()
        {
            return new NotHFA { x = 1f, y = 10f, z = 100 };
        }

        public static Vector2 HFA_Vector2_Return()
        {
            return new Vector2(1f, 10f);
        }

        public static Vector3 HFA_Vector3_Return()
        {
            return new Vector3(1f, 10f, 100f);
        }

        public static Vector4 HFA_Vector4_Return()
        {
            return new Vector4(1f, 10f, 100f, 1000f);
        }

        public static EmbedHFA EmbedHFA_Return()
        {
            return new EmbedHFA { x = new Vector2(1f, 10f), y = 100f};
        }

        public static EmbedHFA2 EmbedHFA2_Return()
        {
            return new EmbedHFA2 { x = new Vector2(1f, 10f), y = new Vector2(100f, 1000f) };
        }

        public static Vector3d EmbedHFA3_Return()
        {
            return new Vector3d { x = 1.0, y = 10.0, z = 100.0};
        }
    }

}
