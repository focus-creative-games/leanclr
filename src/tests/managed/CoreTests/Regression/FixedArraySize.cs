using test;
using System.Collections;
using System.Runtime.InteropServices;


namespace Tests.Bugs
{
    public class FixedArraySize : GeneralTestCaseBase
    {
        unsafe struct FixedBuffer
        {
            public fixed long data[10];
        }

        [UnitTest]
        public unsafe void TestSize()
        {
            int size = sizeof(FixedBuffer);
            Assert.Equal(80, size);
        }

        [StructLayout(LayoutKind.Sequential)]
        unsafe struct FixedBuffer2
        {
            public int length;
            public fixed long data[2];
        }


        [UnitTest]
        public unsafe void TestField()
        {
            var b = new FixedBuffer2();
            Assert.Equal(0, b.length);
            b.length = 1;
            Assert.Equal(1, b.length);
            b.data[1] = 10;
            Assert.Equal(10, b.data[1]);
        }

        [UnitTest]
        public unsafe void TestField2()
        {
            var c = new FixedBuffer2();
            ref FixedBuffer2 b = ref c;
            Assert.Equal(0, b.length);
            b.length = 1;
            Assert.Equal(1, b.length);
            b.data[1] = 10;
            Assert.Equal(10, b.data[1]);
        }
    }
}