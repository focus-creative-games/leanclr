using AOTDefs;


namespace Test
{
    public class TestAOTVirtualReturn
    {
        public virtual sbyte Return_sbyte_minus1()
        {
            return -1;
        }
        public virtual sbyte Return_sbyte_1()
        {
            return 1;
        }

        public virtual byte Return_byte_1()
        {
            return 1;
        }

        public virtual short Return_short_minus1()
        {
            return -1;
        }

        public virtual short Return_short_1()
        {
            return 1;
        }

        public virtual ushort Return_ushort_1()
        {
            return 1;
        }

        public virtual int Return_int_minus1()
        {
            return -1;
        }

        public virtual int Return_int_1()
        {
            return 1;
        }

        public virtual uint Return_uint_1()
        {
            return 1;
        }

        public virtual long Return_long_minus_0x1122334455667788()
        {
            return -0x1122334455667788L;
        }

        public virtual long Return_long_0x1122334455667788()
        {
            return 0x1122334455667788;
        }

        public virtual ulong Return_ulong_0x1122334455667788()
        {
            return 0x1122334455667788UL;
        }


        public virtual float Return_float_1()
        {
            return 1f;
        }
        public virtual double Return_double_1()
        {
            return 1f;
        }

        public virtual AOT_Enum_sbyte Return_sbyte_enum_A()
        {
            return AOT_Enum_sbyte.A;
        }

        public virtual AOT_Enum_sbyte Return_sbyte_enum_B()
        {
            return AOT_Enum_sbyte.B;
        }

        public virtual AOT_Enum_byte Return_byte_enum_A()
        {
            return AOT_Enum_byte.A;
        }

        public virtual AOT_Enum_short Return_short_enum_A()
        {
            return AOT_Enum_short.A;
        }

        public virtual AOT_Enum_short Return_short_enum_B()
        {
            return AOT_Enum_short.B;
        }

        public virtual AOT_Enum_ushort Return_ushort_enum_A()
        {
            return AOT_Enum_ushort.A;
        }

        public virtual AOT_Enum_int Return_int_enum_A()
        {
            return AOT_Enum_int.A;
        }

        public virtual AOT_Enum_int Return_int_enum_B()
        {
            return AOT_Enum_int.B;
        }

        public virtual AOT_Enum_long Return_long_enum_A()
        {
            return AOT_Enum_long.A;
        }

        public virtual AOT_Enum_long Return_long_enum_B()
        {
            return AOT_Enum_long.B;
        }

        public virtual AOT_Enum_ulong Return_ulong_enum_A()
        {
            return AOT_Enum_ulong.A;
        }

        public virtual Vector2 Return_Vector2_1_2()
        {
            return new Vector2(1, 2);
        }

        public virtual Vector3 Return_Vector3_1_2_3()
        {
            return new Vector3(1, 2, 3);
        }

        public virtual Vector4 Return_Vector4_1_2_3_4()
        {
            return new Vector4(1, 2, 3, 4);
        }


        public virtual ValueTypeSize2 Return_ValueTypeSize2()
        {
            return new ValueTypeSize2 { x1 = 1, x2 = 2 };
        }

        public virtual ValueTypeSize3 Return_ValueTypeSize3()
        {
            return new ValueTypeSize3 { x1 = 1, x2 = 2, x3 = 3 };
        }

        public virtual ValueTypeSize4 Return_ValueTypeSize4()
        {
            return new ValueTypeSize4 { x1 = 1 };
        }

        public virtual ValueTypeSize5 Return_ValueTypeSize5()
        {
            return new ValueTypeSize5 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
        }

        public virtual ValueTypeSize8 Return_ValueTypeSize8()
        {
            return new ValueTypeSize8 { x1 = 1 };
        }

        public virtual ValueTypeSize9 Return_ValueTypeSize9()
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

        public virtual string Return_string_abc()
        {
            return "abc";
        }
    }
}
