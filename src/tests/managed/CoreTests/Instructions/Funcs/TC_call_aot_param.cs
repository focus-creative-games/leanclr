
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
    internal class TC_call_aot_param : GeneralTestCaseBase
    {
        [UnitTest]
        public void sbyte_1()
        {
            Assert.Equal(-1, TestAOTParam.Sbyte_ReturnParam(-1));
            Assert.Equal(1, TestAOTParam.Sbyte_ReturnParam(1));
        }

        [UnitTest]
        public void short_1()
        {
            Assert.Equal(-1, TestAOTParam.Short_ReturnParam(-1));
            Assert.Equal(1, TestAOTParam.Short_ReturnParam(1));
        }

        [UnitTest]
        public void int_1()
        {
            Assert.Equal(-1, TestAOTParam.Int_ReturnParam(-1));
            Assert.Equal(1, TestAOTParam.Int_ReturnParam(1));
        }


        [UnitTest]
        public void long_1()
        {
            Assert.Equal(-1, TestAOTParam.Long_ReturnParam(-1));
            Assert.Equal(1, TestAOTParam.Long_ReturnParam(1));
        }

        [UnitTest]
        public void float_1()
        {
            Assert.Equal(1f, TestAOTParam.Float_ReturnParam(1f));
        }

        [UnitTest]
        public void double_1()
        {
            Assert.Equal(1.0, TestAOTParam.Double_ReturnParam(1.0));
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            Assert.Equal(-1L, TestAOTParam.SbyteEnum_ReturnParam(AOT_Enum_sbyte.A));
            Assert.Equal(1L, TestAOTParam.SbyteEnum_ReturnParam(AOT_Enum_sbyte.B));
        }

        [UnitTest]
        public void enum_short_1()
        {
            Assert.Equal(-1L, TestAOTParam.ShortEnum_ReturnParam(AOT_Enum_short.A));
            Assert.Equal(1L, TestAOTParam.ShortEnum_ReturnParam(AOT_Enum_short.B));
        }


        [UnitTest]
        public void enum_int_1()
        {
            Assert.Equal(-1L, TestAOTParam.IntEnum_ReturnParam(AOT_Enum_int.A));
            Assert.Equal(1L, TestAOTParam.IntEnum_ReturnParam(AOT_Enum_int.B));
        }


        [UnitTest]
        public void enum_long_1()
        {
            Assert.Equal(-0x1122334455667788L, TestAOTParam.LongEnum_ReturnParam(AOT_Enum_long.A));
            Assert.Equal(0x1122334455667788L, TestAOTParam.LongEnum_ReturnParam(AOT_Enum_long.B));
        }

        [UnitTest]
        public void valuetype_1()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize1_ReturnParam(new ValueTypeSize1 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_4()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize4_ReturnParam(new ValueTypeSize4 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_5()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize5_ReturnParam(new ValueTypeSize5 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_8()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize8_ReturnParam(new ValueTypeSize8 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_9()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize9_ReturnParam(new ValueTypeSize9 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_16()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize16_ReturnParam(new ValueTypeSize16 { x1 = 1 }));
        }

        [UnitTest]
        public void valuetype_20()
        {
            Assert.Equal(1L, TestAOTParam.ValueTypeSize20_ReturnParam(new ValueTypeSize20 { x1 = 1 }));
        }

        [UnitTest]
        public void vector2()
        {
            Assert.Equal(3f, TestAOTParam.HFA_Vector2_ReturnParam(new Vector2(1f, 2f)));
        }

        [UnitTest]
        public void vector3()
        {
            Assert.Equal(6f, TestAOTParam.HFA_Vector3_ReturnParam(new Vector3(1f, 2f, 3f)));
        }

        [UnitTest]
        public void vector4()
        {
            Assert.Equal(10f, TestAOTParam.HFA_Vector4_ReturnParam(new Vector4(1f, 2f, 3f, 4f)));
        }

        [UnitTest]
        public void embedHFA()
        {
            var a = TestAOTParam.EmbedHFA_ReturnParam(new EmbedHFA { x = new Vector2(1f, 10f), y = 100f });
            Assert.Equal(111f, a);
        }

        [UnitTest]
        public void embedHFA2()
        {
            var a = TestAOTParam.EmbedHFA2_ReturnParam(new EmbedHFA2 { x = new Vector2(1f, 10f), y = new Vector2(100f, 1000f) });
            Assert.Equal(1111f, a);
        }

        [UnitTest]
        public void embedHFA3()
        {
            var a = TestAOTParam.EmbedHFA3_ReturnParam(new Vector3d() { x = 1, y = 2, z = 3});
            Assert.Equal(6.0, a);
        }
    }
}
