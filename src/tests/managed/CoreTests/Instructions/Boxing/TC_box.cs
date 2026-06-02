
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Boxs
{
    internal class TC_box : GeneralTestCaseBase
    {

        public static object Box<T>(T a)
        {
            return a;
        }

        [UnitTest]
        public void int_1()
        {
            var o = (object)1;
            Assert.NotNull(o);
            Assert.Equal("Int32", o.GetType().Name);
            Assert.Equal("1", o.ToString());
        }

        [UnitTest]
        public void int_null_1()
        {
            int? x = 1;
            var o = (object)x;
            Assert.NotNull(o);
            Assert.Equal("1", o.ToString());
        }

        [UnitTest]
        public void int_null_2()
        {
            int? x = null;
            var o = (object)x;
            Assert.Null(o);
        }

        [UnitTest]
        public void enum_byte_1()
        {
            AOT_Enum_sbyte x = AOT_Enum_sbyte.A;
            var o = (object)x;
            Assert.NotNull(o);
            Assert.Equal("A", o.ToString());
        }

        [UnitTest]
        public void ValueTypeSize1_1()
        {
            var x = new ValueTypeSize1() { x1 = 1};
            var o = (object)x;
            Assert.NotNull(o);
            Assert.Equal("ValueTypeSize1", o.GetType().Name);
            Assert.Equal("x1=1", o.ToString());
        }

        [UnitTest]
        public void object_1()
        {
            var x = new object();
            var y = Box(x);
            Assert.True(ReferenceEquals(x, y));
        }
    }
}
