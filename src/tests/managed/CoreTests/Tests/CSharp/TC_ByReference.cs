using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    internal class TC_ByReference : GeneralTestCaseBase
    {
        [UnitTest]
        public void SpanValue()
        {
            var array = new MyStruct[] { new MyStruct(1), new MyStruct(2), new MyStruct(3) };
            var sum = 0;
            foreach (ref readonly var s in array.AsSpan())
            {
                sum += s.Value;
            }
            Assert.Equal(6, sum);
        }

        private struct MyStruct
        {
            public readonly int Value;

            public MyStruct(int value)
            {
                Value = value;
            }
        }
    }
}
