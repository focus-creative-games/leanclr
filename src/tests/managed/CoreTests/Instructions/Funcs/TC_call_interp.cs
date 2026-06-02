using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
    internal class TC_call_interp : GeneralTestCaseBase
    {
        //[UnitTest]
        //public void class_null_this()
        //{
        //    ForInhereAotClass a = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    a.Show2();
        //    Assert.Fail();
        //}

        [UnitTest]
        public void class_1()
        {
            var a = new ForInhereAotClass(1, 2);
            Assert.Equal(1, a.Show());
        }

        [UnitTest]
        public void class_2()
        {
            var a = new ForInhereAotClass(1, 2);
            a.Inc();
            Assert.Equal(2, a.a);
        }

        [UnitTest]
        public void class_static_1()
        {
            Assert.Equal(10, ForInhereAotClass.Default());
        }

        [UnitTest]
        public void class_generic_1()
        {
            var a = new ForInhereAotClass(1, 2);
            Assert.Equal(100, a.CallGeneric(1));
        }

        [UnitTest]
        public void class_static_generic_1()
        {
            Assert.Equal(100, ForInhereAotClass.CallStaticGeneric(100));
        }

        [UnitTest]
        public void struct_1()
        {
            var a = new ForInhereAotValue(1, 2);
            Assert.Equal(1, a.Show());
        }

        [UnitTest]
        public void struct_2()
        {
            var a = new ForInhereAotValue(1, 2);
            a.Inc();
            Assert.Equal(2, a.a);
        }

        [UnitTest]
        public void struct_static_1()
        {
            Assert.Equal(10, ForInhereAotValue.Default());
        }

        [UnitTest]
        public void struct_generic_1()
        {
            var a = new ForInhereAotValue(1, 2);
            Assert.Equal(100, a.CallGeneric(100));
        }

        [UnitTest]
        public void struct_static_generic_1()
        {
            Assert.Equal(100, ForInhereAotValue.CallStaticGeneric(100));
        }

        [UnitTest]
        public void LocalFunction_Call_1_Param_Return2()
        {
            //出错在 MetadataParser.cpp 524行 IL2CPP_ASSERT(paramCount == methodDef.parameterCount);
            //B的函数定义只有一个参数 编译成il后会有两个参数
            int c = 1;
            int B(int d)
            {
                return d + c;
            }

            Assert.Equal(2, B(1));
        }
    }

}
