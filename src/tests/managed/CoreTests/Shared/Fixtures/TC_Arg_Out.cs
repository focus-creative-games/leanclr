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
    public static class TC_Arg_Out
    {


        public static int Arg_bool(out bool x)
        {
            x = true;
            return 1;
        }

        public static int Arg_byte(out byte x)
        {
            x = 1;
            return x;
        }

        public static int Arg_sbyte(out sbyte x)
        {
            x = -1;
            return x;
        }

        public static int Arg_ushort(out ushort x)
        {
            x = 1;
            return x;
        }

        public static int Arg_short(out short x)
        {
            x = -1;
            return x;
        }

        public static int Arg_char(out char x)
        {
            x = 'a';
            return x;
        }

        public static uint Arg_uint(out uint x)
        {
            x = 1;
            return x;
        }

        public static int Arg_int(out int x)
        {
            x = -1;
            return x;
        }

        public static ulong Arg_ulong(out ulong x)
        {
            x = 1;
            return x;
        }

        public static long Arg_long(out long x)
        {
            x = -1;
            return x;
        }

        public static string Arg_string(out string x)
        {
            x = "abc";
            return x;
        }

        // enum


        public static int Arg_enum_byte(out AOT_Enum_byte x)
        {
            x = AOT_Enum_byte.A;
            return (int)x;
        }

        public static int Arg_enum_sbyte(out AOT_Enum_sbyte x)
        {
            x = AOT_Enum_sbyte.A;
            return (int)x;
        }

        public static int Arg_enum_ushort(out AOT_Enum_ushort x)
        {
            x = AOT_Enum_ushort.A;
            return (int)x;
        }

        public static int Arg_enum_short(out AOT_Enum_short x)
        {
            x = AOT_Enum_short.A;
            return (int)x;
        }

        public static uint Arg_enum_uint(out AOT_Enum_uint x)
        {
            x = AOT_Enum_uint.A;
            return (uint)x;
        }

        public static int Arg_enum_int(out AOT_Enum_int x)
        {
            x = AOT_Enum_int.A;
            return (int)x;
        }

        public static ulong Arg_enum_ulong(out AOT_Enum_ulong x)
        {
            x = AOT_Enum_ulong.A;
            return (ulong)x;
        }

        public static long Arg_enum_long(out AOT_Enum_long x)
        {
            x = AOT_Enum_long.A;
            return (long)x;
        }

        public static ValueTypeSize0 Arg_ValueTypeSize0(out ValueTypeSize0 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize1 Arg_ValueTypeSize1(out ValueTypeSize1 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize2 Arg_ValueTypeSize2(out ValueTypeSize2 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize3 Arg_ValueTypeSize3(out ValueTypeSize3 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize4 Arg_ValueTypeSize4(out ValueTypeSize4 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize5 Arg_ValueTypeSize5(out ValueTypeSize5 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize8 Arg_ValueTypeSize8(out ValueTypeSize8 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize9 Arg_ValueTypeSize9(out ValueTypeSize9 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize16 Arg_ValueTypeSize16(out ValueTypeSize16 x)
        {
            x = default;
            return x;
        }

        public static ValueTypeSize20 Arg_ValueTypeSize20(out ValueTypeSize20 x)
        {
            x = default;
            return x;
        }
    }
}
