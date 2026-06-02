using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Tests.Bugs
{
    public unsafe class MyClass<T> where T : unmanaged
    {
        private int offset;

        public MyClass(int offset)
        {
            this.offset = offset;
        }

        public T GetValue(byte* mem)
        {
            return *(T*)(mem + offset);
        }
    }

    internal unsafe class UnmanagedAccess : GeneralTestCaseBase
    {
        [UnitTest]
        public void FalseReturnTrue()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(int*)arr = 0x01020000;
                var c = new MyClass<bool>(1);
                if (c.GetValue(arr))
                {
                    Assert.Fail("fail");
                }
            }
        }

        [UnitTest]
        public void ByteZeroAsOther()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(int*)arr = 0x01020000;
                var c = new MyClass<byte>(1);
                int x = c.GetValue(arr);
                Assert.Equal(0, x);
            }
        }


        [UnitTest]
        public void SbyteZeroAsOther()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(int*)arr = 0x01020000;
                var c = new MyClass<sbyte>(1);
                int x = c.GetValue(arr);
                Assert.Equal(0, x);
            }
        }


        [UnitTest]
        public void UshortZeroAsOther()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(int*)arr = 0x01020000;
                var c = new MyClass<ushort>(0);
                int x = c.GetValue(arr);
                Assert.Equal(0, x);
            }
        }

        [UnitTest]
        public void ShortZeroAsOther()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(int*)arr = 0x01020000;
                var c = new MyClass<short>(0);
                int x = c.GetValue(arr);
                Assert.Equal(0, x);
            }
        }

        [UnitTest]
        public void IntZeroAsOther()
        {
            var bytes = new byte[30];
            fixed (byte* arr = bytes)
            {
                *(long*)arr = 0x0100000000;
                var c = new MyClass<short>(0);
                int x = c.GetValue(arr);
                Assert.Equal(0, x);
            }
        }
    }
}
