using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    internal class TC_localloc: GeneralTestCaseBase
    {
        [UnitTest]
        public unsafe void alloc_0()
        {
            int* b = stackalloc int[0];
            Assert.Equal(null, b);
        }

        [UnitTest]
        public unsafe void alloc_1()
        {
            int* b = stackalloc int[3];
            Assert.Equal(0, b[0]);
            Assert.Equal(0, b[1]);
            Assert.Equal(0, b[2]);
            b[0] = 1;
            b[1] = 2;
            b[2] = 3;
            Assert.Equal(1, b[0]);
        }
        
        [UnitTest]
        public unsafe void alloc_2()
        {
            int n = 10;
            int* b = stackalloc int[n];
            for (int i = 0; i < n; i++)
            {
                b[i] = 1;
            }           
            int* c = stackalloc int[n];
            for (int i = 0; i < n; i++)
            {
                c[i] = 2;
            }
            for (int i = 0; i < n; i++)
            {
                Assert.Equal(1, b[i]);
            }
        }

        [UnitTest]
        public unsafe void alloc_3()
        {
            byte* b = stackalloc byte[] { 1, 2, 3, 4, 5, 6 };
            Assert.Equal(1, b[0]);
            Assert.Equal(2, b[1]);
            Assert.Equal(3, b[2]);
            Assert.Equal(4, b[3]);
            Assert.Equal(5, b[4]);
            Assert.Equal(6, b[5]);
        }
        
        //[UnitTest]
        //public unsafe void alloc_overflow()
        //{
        //    Assert.ExpectException<StackOverflowException>();
        //    int* b = stackalloc int[1000000000];
        //    b[0] = 5;
        //}
    }
}
