using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace AOTDefs
{
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
        public static ValueTypeSize1 s1 = new ValueTypeSize1 { x1 = 1};
        public static ValueTypeSize2 s2 = new ValueTypeSize2 { x1 = 2};
        public static ValueTypeSize3 s3 = new ValueTypeSize3 { x1 = 3};
        public static ValueTypeSize4 s4 = new ValueTypeSize4 { x1 = 4};
        public static ValueTypeSize5 s5 = new ValueTypeSize5 { x1 = 5};
        public static ValueTypeSize8 s8 = new ValueTypeSize8 { x1 = 6};
        public static ValueTypeSize9 s9 = new ValueTypeSize9 { x1 = 7};
        public static ValueTypeSize16 s16 = new ValueTypeSize16 { x1 = 8};

        public static AOT_Enum_byte e1 = AOT_Enum_byte.A;
        public static AOT_Enum_sbyte e2 = AOT_Enum_sbyte.B;
        public static AOT_Enum_short e3 = AOT_Enum_short.A;
        public static AOT_Enum_ushort e4 = AOT_Enum_ushort.A;
        public static AOT_Enum_int e5 = AOT_Enum_int.B;
        public static AOT_Enum_uint e6 = AOT_Enum_uint.A;
        public static AOT_Enum_long e7 = AOT_Enum_long.B;
        public static AOT_Enum_ulong e8 = AOT_Enum_ulong.A;
    }
}
