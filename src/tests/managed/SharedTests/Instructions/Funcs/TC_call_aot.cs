
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
    internal class TC_call_aot : GeneralTestCaseBase
    {
        //[UnitTest]
        //public void class_null_this()
        //{
        //    ForFunClass a = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    a.Show2();
        //    Assert.Fail();
        //}

        [UnitTest]
        public void class_1()
        {
            var a = new ForFunClass(1, 2);
            Assert.Equal(1, a.Show());
        }

        [UnitTest]
        public void class_2()
        {
            var a = new ForFunClass(1, 2);
            a.Inc();
            Assert.Equal(2, a.a);
        }

        [UnitTest]
        public void class_generic_1()
        {
            var a = new ForFunClass(1, 2);
            Assert.Equal("100-1", a.CallGeneric(100));
        }

        [UnitTest]
        public void class_static_1()
        {
            Assert.Equal(10, ForFunClass.Default());
        }

        [UnitTest]
        public void class_static_generic_1()
        {
            Assert.Equal(100, ForFunClass.CallStaticGeneric(100));
        }

        [UnitTest]
        public void struct_1()
        {
            var a = new ForFunValue(1, 2);
            Assert.Equal(1, a.Show());
        }

        [UnitTest]
        public void struct_2()
        {
            var a = new ForFunValue(1, 2);
            a.Inc();
            Assert.Equal(2, a.a);
        }

        [UnitTest]
        public void struct_static_1()
        {
            Assert.Equal(10, ForFunValue.Default());
        }

        [UnitTest]
        public void struct_generic_1()
        {
            var a = new ForFunValue(1, 2);
            Assert.Equal(100, a.CallGeneric(100));
        }

        [UnitTest]
        public void struct_static_generic_1()
        {
            Assert.Equal(100, ForFunValue.CallStaticGeneric(100));
        }
    }
}
