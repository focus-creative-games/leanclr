using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    internal class TC_Interlocked : GeneralTestCaseBase
    {

        [UnitTest]
        public void Exchange_intptr()
        {
            IntPtr a = (IntPtr)1;
            Interlocked.Exchange(ref a, (IntPtr)2);
            Assert.Equal((IntPtr)2, a);
        }

        [UnitTest]
        public void Exchange_object()
        {
            string a = "a";
            Interlocked.Exchange(ref a, "b");
            Assert.Equal("b", a);
        }


        [UnitTest]
        public void Exchange_int()
        {
            int a = 1;
            Interlocked.Exchange(ref a, 2);
            Assert.Equal(2, a);
        }

        [UnitTest]
        public void Exchange_float()
        {
            float a = 1f;
            Interlocked.Exchange(ref a, 2f);
            Assert.Equal(2f, a);
        }

        [UnitTest]
        public void Exchange_long()
        {
            long a = 0x1122334455L;
            Interlocked.Exchange(ref a, -0x1122334455L);
            Assert.Equal(-0x1122334455L, a);
        }

        [UnitTest]
        public void Exchange_double()
        {
            double a = 1.0;
            Interlocked.Exchange(ref a, 2.0);
            Assert.Equal(2.0, a);
        }

        [UnitTest]
        public void CompareExchange_int()
        {
            int a = 1;
            int b = Interlocked.CompareExchange(ref a, 10, 1);
            Assert.Equal(10, a);
            Assert.Equal(1, b);

            int c = Interlocked.CompareExchange(ref a, 100, 2);
            Assert.Equal(10, a);
            Assert.Equal(10, c);
        }

        [UnitTest]
        public void CompareExchange_float()
        {
            float a = 1f;
            float b = Interlocked.CompareExchange(ref a, 10f, 1f);
            Assert.Equal(10f, a);
            Assert.Equal(1f, b);

            float c = Interlocked.CompareExchange(ref a, 100f, 2f);
            Assert.Equal(10f, a);
            Assert.Equal(10f, c);
        }

        [UnitTest]
        public void CompareExchange_long()
        {
            long a = 1;
            long b = Interlocked.CompareExchange(ref a, 10, 1);
            Assert.Equal(10, a);
            Assert.Equal(1, b);

            long c = Interlocked.CompareExchange(ref a, 100, 2);
            Assert.Equal(10, a);
            Assert.Equal(10, c);
        }

        [UnitTest]
        public void CompareExchange_double()
        {
            double a = 1;
            double b = Interlocked.CompareExchange(ref a, 10, 1);
            Assert.Equal(10, a);
            Assert.Equal(1, b);

            double c = Interlocked.CompareExchange(ref a, 100, 2);
            Assert.Equal(10, a);
            Assert.Equal(10, c);
        }

        [UnitTest]
        public void CompareExchange_object()
        {
            string a = "a";
            string b = Interlocked.CompareExchange(ref a, "b", a);
            Assert.Equal("b", a);
            Assert.Equal("a", b);

            string c = Interlocked.CompareExchange(ref a, "c", b);
            Assert.Equal("b", a);
            Assert.Equal("b", c);
        }
    }
}
