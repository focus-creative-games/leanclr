using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
#if IL2CPP_ONLY
    internal class TC_System_Runtime_InteropServices_Marshal : GeneralTestCaseBase
    {
        [UnitTest]
        public void AllocHGlobal_ok()
        {
            IntPtr ptr = System.Runtime.InteropServices.Marshal.AllocHGlobal(100);
            Assert.True(ptr != IntPtr.Zero);
            System.Runtime.InteropServices.Marshal.FreeHGlobal(ptr);
        }

        [UnitTest]
        public void AllocCoTaskMemory_ok()
        {
            IntPtr ptr = System.Runtime.InteropServices.Marshal.AllocCoTaskMem(100);
            Assert.True(ptr != IntPtr.Zero);
            System.Runtime.InteropServices.Marshal.FreeCoTaskMem(ptr);
        }

        [UnitTest]
        public unsafe void PtrToStringAnsi_ok()
        {
            string str = "Hello";
            IntPtr ptr = System.Runtime.InteropServices.Marshal.StringToHGlobalAnsi(str);
            Assert.True(ptr != IntPtr.Zero);
            Assert.Equal((byte)'H', (*(byte*)ptr));
            Assert.Equal((byte)'o', (*((byte*)ptr + 4)));
            string result = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptr);
            Assert.Equal(str, result);
            System.Runtime.InteropServices.Marshal.FreeHGlobal(ptr);
        }

        [UnitTest]
        public unsafe void PtrToStringUni_ok()
        {
            string str = "Hello";
            IntPtr ptr = System.Runtime.InteropServices.Marshal.StringToHGlobalUni(str);
            Assert.True(ptr != IntPtr.Zero);
            Assert.Equal((ushort)'H', (*(ushort*)ptr));
            Assert.Equal((ushort)'o', (*((ushort*)ptr + 4)));
            string result = System.Runtime.InteropServices.Marshal.PtrToStringUni(ptr);
            Assert.Equal(str, result);
            System.Runtime.InteropServices.Marshal.FreeHGlobal(ptr);
        }

        [UnitTest]
        public unsafe void PtrToStringBSTR()
        {
            string str = "Hello";
            IntPtr ptr = System.Runtime.InteropServices.Marshal.StringToBSTR(str);
            Assert.True(ptr != IntPtr.Zero);
            Assert.Equal((ushort)'H', (*(ushort*)ptr));
            Assert.Equal((ushort)'o', (*((ushort*)ptr + 4)));
            string result = System.Runtime.InteropServices.Marshal.PtrToStringBSTR(ptr);
            Assert.Equal(str, result);
            System.Runtime.InteropServices.Marshal.FreeBSTR(ptr);
        }

        [UnitTest]
        public void SizeOf_ok()
        {
            int sizeInt = System.Runtime.InteropServices.Marshal.SizeOf(typeof(int));
            Assert.Equal(4, sizeInt);
            int sizeDouble = System.Runtime.InteropServices.Marshal.SizeOf(typeof(double));
            Assert.Equal(8, sizeDouble);
        }

        public class A
        {
            public int x;
            public int y;

            public A()
            {
                x = 1;
                y = 2;
            }

            public int Sum()
            {
                return x + y;
            }
        }

        [UnitTest]
        public void OffsetOf_int()
        {
            int offset = Marshal.OffsetOf(typeof(A), "y").ToInt32();
            Assert.Equal(4, offset);
        }

        [UnitTest]
        public void UnsafeAddrOfPinnedArrayElement_ok()
        {
            int[] arr = new int[] { 10, 20, 30, 40, 50 };
            IntPtr ptr = Marshal.UnsafeAddrOfPinnedArrayElement(arr, 2);
            unsafe
            {
                int* p = (int*)ptr;
                Assert.Equal(30, *p);
            }
        }
    }
#endif
}
