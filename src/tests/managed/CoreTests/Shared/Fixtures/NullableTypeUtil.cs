using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public static class NullableTypeUtil
    {
        public static bool HasValue(bool? x)
        {
            return x.HasValue;
        }

        public static bool GetValue(bool? x)
        {
            return x.Value;
        }

        public static bool HasValue(byte? x)
        {
            return x.HasValue;
        }

        public static int GetValue(byte? x)
        {
            return x.Value;
        }

        public static bool HasValue(short? x)
        {
            return x.HasValue;
        }

        public static int GetValue(short? x)
        {
            return x.Value;
        }

        public static bool HasValue(int? x)
        {
            return x.HasValue;
        }

        public static int GetValue(int? x)
        {
            return x.Value;
        }

        public static bool HasValue(long? x)
        {
            return x.HasValue;
        }

        public static long GetValue(long? x)
        {
            return x.Value;
        }

        public static bool HasValue(AOT_Enum_byte? x)
        {
            return x.HasValue;
        }

        public static AOT_Enum_byte GetValue(AOT_Enum_byte? x)
        {
            return x.Value;
        }

        public static bool HasValue(AOT_Enum_short? x)
        {
            return x.HasValue;
        }

        public static AOT_Enum_short GetValue(AOT_Enum_short? x)
        {
            return x.Value;
        }

        public static bool HasValue(AOT_Enum_int? x)
        {
            return x.HasValue;
        }

        public static AOT_Enum_int GetValue(AOT_Enum_int? x)
        {
            return x.Value;
        }

        public static bool HasValue(AOT_Enum_long? x)
        {
            return x.HasValue;
        }

        public static AOT_Enum_long GetValue(AOT_Enum_long? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize1? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize1 GetValue(ValueTypeSize1? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize2? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize2 GetValue(ValueTypeSize2? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize3? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize3 GetValue(ValueTypeSize3? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize4? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize4 GetValue(ValueTypeSize4? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize5? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize5 GetValue(ValueTypeSize5? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize8? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize8 GetValue(ValueTypeSize8? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize9? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize9 GetValue(ValueTypeSize9? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize16? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize16 GetValue(ValueTypeSize16? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTypeSize20? x)
        {
            return x.HasValue;
        }

        public static ValueTypeSize20 GetValue(ValueTypeSize20? x)
        {
            return x.Value;
        }

        public static bool HasValue(ValueTuple<int, int, int>? x)
        {
            return x.HasValue;
        }

        public static ValueTuple<int, int, int> GetValue(ValueTuple<int, int, int>? x)
        {
            return x.Value;
        }
    }
}
