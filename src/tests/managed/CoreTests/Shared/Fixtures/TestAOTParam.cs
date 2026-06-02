using AOTDefs;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Test
{

    public static class TestAOTParam
    {
        public static long Byte_ReturnParam(byte x)
        {
            return x;
        }

        public static long Sbyte_ReturnParam(sbyte x)
        {
            return x;
        }

        public static long Short_ReturnParam(short x)
        {
            return x;
        }

        public static long Ushort_ReturnParam(ushort x)
        {
            return x;
        }


        public static long Int_ReturnParam(int x)
        {
            return x;
        }

        public static long Uint_ReturnParam(uint x)
        {
            return x;
        }

        public static long Long_ReturnParam(long x)
        {
            return x;
        }

        public static ulong Ulong_ReturnParam(ulong x)
        {
            return x;
        }

        public static float Float_ReturnParam(float x)
        {
            return x;
        }

        public static double Double_ReturnParam(double x)
        {
            return x;
        }

        public static long ByteEnum_ReturnParam(AOT_Enum_byte x)
        {
            return (long)x;
        }

        public static long SbyteEnum_ReturnParam(AOT_Enum_sbyte x)
        {
            return (long)x;
        }

        public static long ShortEnum_ReturnParam(AOT_Enum_short x)
        {
            return (long)x;
        }

        public static long UshortEnum_ReturnParam(AOT_Enum_ushort x)
        {
            return (long)x;
        }

        public static long IntEnum_ReturnParam(AOT_Enum_int x)
        {
            return (long)x;
        }

        public static long UintEnum_ReturnParam(AOT_Enum_uint x)
        {
            return (long)x;
        }

        public static long LongEnum_ReturnParam(AOT_Enum_long x)
        {
            return (long)x;
        }

        public static ulong UlongEnum_ReturnParam(AOT_Enum_ulong x)
        {
            return (ulong)x;
        }

        public static long ValueTypeSize1_ReturnParam(ValueTypeSize1 x)
        {
            return x.x1;
        }

        public static long ValueTypeSize4_ReturnParam(ValueTypeSize4 x)
        {
            return x.x1;
        }

        public static long ValueTypeSize5_ReturnParam(ValueTypeSize5 x)
        {
            return x.x1 + x.x5;
        }

        public static long ValueTypeSize8_ReturnParam(ValueTypeSize8 x)
        {
            return x.x1;
        }

        public static long ValueTypeSize9_ReturnParam(ValueTypeSize9 x)
        {
            return x.x1 + x.x9;
        }

        public static long ValueTypeSize16_ReturnParam(ValueTypeSize16 x)
        {
            return x.x1 + x.x2;
        }

        public static long ValueTypeSize20_ReturnParam(ValueTypeSize20 x)
        {
            return x.x1 + x.x5;
        }

        public static float NotHFA_ReturnParam(NotHFA x)
        {
            return x.x + x.y + x.z;
        }

        public static float HFA_Vector2_ReturnParam(Vector2 x)
        {
            return x.x + x.y;
        }

        public static float HFA_Vector3_ReturnParam(Vector3 x)
        {
            return x.x + x.y + x.z;
        }

        public static float HFA_Vector4_ReturnParam(Vector4 x)
        {
            return x.x + x.y + x.z + x.w;
        }

        public static float EmbedHFA_ReturnParam(EmbedHFA x)
        {
            return x.x.x + x.x.y + x.y;
        }

        public static float EmbedHFA2_ReturnParam(EmbedHFA2 x)
        {
            return x.x.x + x.x.y + x.y.x + x.y.y;
        }

        public static double EmbedHFA3_ReturnParam(Vector3d x)
        {
            return x.x + x.y + x.z;
        }
    }

}
