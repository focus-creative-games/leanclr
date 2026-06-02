
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;


namespace Tests.Instruments.Funcs
{
    /// <summary>
    /// 主要考虑几个情况
    /// 1. aot or interpreter
    /// 2. class与struct类型
    /// </summary>
    internal class TC_newobj_aot : GeneralTestCaseBase
    {
        [UnitTest]
        public void class_aot()
        {
            var a = new ForFunClass(1, 2);
            Assert.NotNull(a);
            Assert.Equal(1, a.a);
            Assert.Equal(2, a.b);
        }

        [UnitTest]
        public void struct_aot_1()
        {
            var a = new ForFunValue(1, 2);
            Assert.Equal(1, a.a);
            Assert.Equal(2, a.b);
        }

        [UnitTest]
        public void struct_aot_2()
        {
            var a = new ForFunValue();
            Assert.Equal(0, a.a);
            Assert.Equal(0, a.b);
        }

        [UnitTest]
        public void sbyte_1()
        {
            var a = new AOTCtor_sbyte(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_sbyte(1);
            Assert.Equal(1, b.x);
        }

        [UnitTest]
        public void short_1()
        {
            var a = new AOTCtor_short(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_short(1);
            Assert.Equal(1, b.x);
        }

        [UnitTest]
        public void int_1()
        {
            var a = new AOTCtor_int(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_int(1);
            Assert.Equal(1, b.x);
        }


        [UnitTest]
        public void long_1()
        {
            var a = new AOTCtor_long(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_long(1);
            Assert.Equal(1, b.x);
        }

        [UnitTest]
        public void float_1()
        {
            var a = new AOTCtor_float(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_float(1);
            Assert.Equal(1, b.x);
        }

        [UnitTest]
        public void double_1()
        {
            var a = new AOTCtor_double(-1);
            Assert.Equal(-1, a.x);
            var b = new AOTCtor_double(1);
            Assert.Equal(1, b.x);
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            var a = new AOTCtor_enum_sbyte(AOT_Enum_sbyte.A);
            Assert.Equal(AOT_Enum_sbyte.A, a.x);
            var b = new AOTCtor_enum_sbyte(AOT_Enum_sbyte.B);
            Assert.Equal(AOT_Enum_sbyte.B, b.x);
        }

        [UnitTest]
        public void enum_short_1()
        {
            var a = new AOTCtor_enum_short(AOT_Enum_short.A);
            Assert.Equal(AOT_Enum_short.A, a.x);
            var b = new AOTCtor_enum_short(AOT_Enum_short.B);
            Assert.Equal(AOT_Enum_short.B, b.x);
        }


        [UnitTest]
        public void enum_int_1()
        {
            var a = new AOTCtor_enum_int(AOT_Enum_int.A);
            Assert.Equal(AOT_Enum_int.A, a.x);
            var b = new AOTCtor_enum_int(AOT_Enum_int.B);
            Assert.Equal(AOT_Enum_int.B, b.x);
        }


        [UnitTest]
        public void enum_long_1()
        {
            var a = new AOTCtor_enum_long(AOT_Enum_long.A);
            Assert.Equal(AOT_Enum_long.A, a.x);
            var b = new AOTCtor_enum_long(AOT_Enum_long.B);
            Assert.Equal(AOT_Enum_long.B, b.x);
        }

        [UnitTest]
        public void valuetype_1()
        {
            var a = new AOTCtor_ValueType_1(new ValueTypeSize1 { x1 = 1});
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_4()
        {
            var a = new AOTCtor_ValueType_4(new ValueTypeSize4 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_5()
        {
            var a = new AOTCtor_ValueType_5(new ValueTypeSize5 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_8()
        {
            var a = new AOTCtor_ValueType_8(new ValueTypeSize8 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_9()
        {
            var a = new AOTCtor_ValueType_9(new ValueTypeSize9 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_16()
        {
            var a = new AOTCtor_ValueType_16(new ValueTypeSize16 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void valuetype_20()
        {
            var a = new AOTCtor_ValueType_20(new ValueTypeSize20 { x1 = 1 });
            Assert.Equal(1, a.x.x1);
        }

        [UnitTest]
        public void vector2()
        {
            var a = new AOTCtor_Vector2(new Vector2 { x = 1f, y = 2f });
            Assert.Equal(1f, a.x.x);
            Assert.Equal(2f, a.x.y);
        }

        [UnitTest]
        public void vector3()
        {
            var a = new AOTCtor_Vector3(new Vector3 { x = 1f, y = 2f, z = 3f });
            //Assert.Equal(1f, a.x.x);
            //Assert.Equal(2f, a.x.y);
            Assert.Equal(3f, a.x.z);
        }

        [UnitTest]
        public void vector4()
        {
            var a = new AOTCtor_Vector4(new Vector4 { x = 1f, y = 2f, z = 3f, w = 4f });
            Assert.Equal(1f, a.x.x);
            Assert.Equal(2f, a.x.y);
            Assert.Equal(3f, a.x.z);
            Assert.Equal(4f, a.x.w);
        }

        [UnitTest]
        public void embedHFA()
        {
            var a = new AOTCtor_EmbedHFA(new EmbedHFA { x = new Vector2(1f, 2f) });
            Assert.Equal(1f, a.x.x.x);
            Assert.Equal(2f, a.x.x.y);
        }

        [UnitTest]
        public void embedHFA2()
        {
            var a = new AOTCtor_EmbedHFA2(new EmbedHFA2 { x = new Vector2(1f, 2f), y = new Vector2(3f, 4f) });
            Assert.Equal(1f, a.x.x.x);
            Assert.Equal(2f, a.x.x.y);
            Assert.Equal(3f, a.x.y.x);
            Assert.Equal(4f, a.x.y.y);
        }

        [UnitTest]
        public void embedHFA3()
        {
            var a = new AOTCtor_EmbedHFA3(new Vector3d { x = 1, y = 2, z = 3 });
            Assert.Equal(1.0, a.x.x);
            Assert.Equal(2.0, a.x.y);
            Assert.Equal(3.0, a.x.z);
        }
    }
}
