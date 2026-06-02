
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Funcs
{
    /// <summary>
    /// 主要考虑几个情况
    /// 1. aot or interpreter
    /// 2. 有返回值和没有返回值
    /// 3. class与struct类型
    /// 4. 静态与成员
    /// 5. 普通与泛型函数
    /// </summary>
    internal class TC_call_aot_return : GeneralTestCaseBase
    {
        [UnitTest]
        public void sbyte_1()
        {
            Assert.Equal(-1, TestAOTReturn.Return_sbyte_minus1());
            Assert.Equal(1, TestAOTReturn.Return_sbyte_1());
        }

        [UnitTest]
        public void short_1()
        {
            Assert.Equal(-1, TestAOTReturn.Return_short_minus1());
            Assert.Equal(1, TestAOTReturn.Return_short_1());
        }

        [UnitTest]
        public void int_1()
        {
            Assert.Equal(-1, TestAOTReturn.Return_int_minus1());
            Assert.Equal(1, TestAOTReturn.Return_int_1());
        }


        [UnitTest]
        public void long_1()
        {
            Assert.Equal(0x1122334455667788L, TestAOTReturn.Return_long_0x1122334455667788());
            Assert.Equal(-0x1122334455667788L, TestAOTReturn.Return_long_minus_0x1122334455667788());
        }

        [UnitTest]
        public void float_1()
        {
            Assert.Equal(1, TestAOTReturn.Return_float_1());
        }

        [UnitTest]
        public void double_1()
        {
            Assert.Equal(1, TestAOTReturn.Return_double_1());
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            Assert.Equal(AOT_Enum_sbyte.A, TestAOTReturn.Return_sbyte_enum_A());
            Assert.Equal(AOT_Enum_sbyte.B, TestAOTReturn.Return_sbyte_enum_B());
        }

        [UnitTest]
        public void enum_short_1()
        {
            Assert.Equal(AOT_Enum_short.A, TestAOTReturn.Return_short_enum_A());
            Assert.Equal(AOT_Enum_short.B, TestAOTReturn.Return_short_enum_B());
        }


        [UnitTest]
        public void enum_int_1()
        {
            Assert.Equal(AOT_Enum_int.A, TestAOTReturn.Return_int_enum_A());
            Assert.Equal(AOT_Enum_int.B, TestAOTReturn.Return_int_enum_B());
        }


        [UnitTest]
        public void enum_long_1()
        {
            Assert.Equal(AOT_Enum_long.A, TestAOTReturn.Return_long_enum_A());
            Assert.Equal(AOT_Enum_long.B, TestAOTReturn.Return_long_enum_B());
        }

        [UnitTest]
        public void valuetype_1()
        {
            var a = TestAOTReturn.Return_ValueTypeSize1();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_4()
        {
            var a = TestAOTReturn.Return_ValueTypeSize4();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_5()
        {
            var a = TestAOTReturn.Return_ValueTypeSize5();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_8()
        {
            var a = TestAOTReturn.Return_ValueTypeSize8();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_9()
        {
            var a = TestAOTReturn.Return_ValueTypeSize9();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_16()
        {
            var a = TestAOTReturn.Return_ValueTypeSize16();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void valuetype_20()
        {
            var a = TestAOTReturn.Return_ValueTypeSize20();
            Assert.Equal(1, a.x1);
        }

        [UnitTest]
        public void vector2()
        {
            var a = TestAOTReturn.Return_Vector2_1_2();
            Assert.Equal(1f, a.x);
            Assert.Equal(2f, a.y);
        }

        [UnitTest]
        public void vector3()
        {
            var a = TestAOTReturn.Return_Vector3_1_2_3();
            Assert.Equal(1f, a.x);
            Assert.Equal(2f, a.y);
            Assert.Equal(3f, a.z);
        }

        [UnitTest]
        public void vector4()
        {
            var a = TestAOTReturn.Return_Vector4_1_2_3_4();
            Assert.Equal(1f, a.x);
            Assert.Equal(2f, a.y);
            Assert.Equal(3f, a.z);
            Assert.Equal(4f, a.w);
        }

        [UnitTest]
        public void embedHFA()
        {
            var a = TestAOTReturn.EmbedHFA_Return();
            Assert.Equal(1f, a.x.x);
            Assert.Equal(10f, a.x.y);
        }

        [UnitTest]
        public void embedHFA2()
        {
            var a = TestAOTReturn.EmbedHFA2_Return();
            Assert.Equal(1f, a.x.x);
            Assert.Equal(10f, a.x.y);
            Assert.Equal(100f, a.y.x);
            Assert.Equal(1000f, a.y.y);
        }

        [UnitTest]
        public void embedHFA3()
        {
            var a = TestAOTReturn.EmbedHFA3_Return();
            Assert.Equal(1.0, a.x);
            Assert.Equal(10.0, a.y);
            Assert.Equal(100.0, a.z);
        }
    }
}
