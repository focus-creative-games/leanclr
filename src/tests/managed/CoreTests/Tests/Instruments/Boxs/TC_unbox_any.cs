
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
    public class BoxAny<T>
    {
        private object _value = new object();

        public T Value => (T)_value;
    }

    internal class TC_unbox_any : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            object o = (byte)1;
            Assert.Equal(1, (byte)o);
        }

        [UnitTest]
        public void sbyte_1()
        {
            object o = (sbyte)1;
            Assert.Equal(1, (sbyte)o);
        }

        [UnitTest]
        public void short_1()
        {
            object o = (short)1;
            Assert.Equal(1, (short)o);
        }

        [UnitTest]
        public void ushort_1()
        {
            object o = (ushort)1;
            Assert.Equal(1, (ushort)o);
        }

        [UnitTest]
        public void char_1()
        {
            object o = 'a';
            Assert.Equal('a', (char)o);
        }

        [UnitTest]
        public void int_1()
        {
            var o = (object)1;
            int b = (int)o;
            Assert.Equal(1, b);
        }

        [UnitTest]
        public void int_2()
        {
            var o = (object)1;
            Assert.ExpectException<InvalidCastException>(() =>
            {
                long b = (long)o;
            });
        }

        [UnitTest]
        public void null_1()
        {
            Assert.ExpectException<NullReferenceException>(() =>
            {
                object o = null;
                int b = (int)o;
                Assert.Fail();
            });
        }

        [UnitTest]
        public void null_2()
        {
            object o = null;
            int? x = (int?)o;
            Assert.False(x.HasValue);
        }

        [UnitTest]
        public void int_3()
        {
            object o = 1;
            int? x = (int?)o;
            Assert.True(x.HasValue);
            Assert.Equal(1, x.Value);
        }

        [UnitTest]
        public void int_4()
        {
            object o = 1;
            Assert.ExpectException<InvalidCastException>(() =>
            {
                long? x = (long?)o;
            });
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            AOT_Enum_sbyte x = AOT_Enum_sbyte.A;
            var o = (object)x;
            bool b = (AOT_Enum_sbyte)o == AOT_Enum_sbyte.A;
            Assert.True(b);
        }

        [UnitTest]
        public void enum_sbyte_2()
        {
            AOT_Enum_sbyte x = AOT_Enum_sbyte.A;
            var o = (object)x;
            int a = (sbyte)o;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void valuetype_1()
        {
            var x = new ValueTypeSize1() { x1 = 1 };
            var o = (object)x;
            var y = (ValueTypeSize1)o;
            Assert.Equal(1, y.x1);
        }

        [UnitTest]
        public void object_1()
        {
            var x = new BoxAny<List<int>>();
            Assert.ExpectException<InvalidCastException>(() =>
            {
                var y = x.Value;
            });
        }
    }
}
