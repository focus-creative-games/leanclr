using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace AOTDefs
{
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
}
