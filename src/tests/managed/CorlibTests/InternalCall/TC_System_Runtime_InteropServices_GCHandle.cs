using System;
using System.Runtime.InteropServices;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Runtime_InteropServices_GCHandle : TestCaseBase
    {
        [UnitTest]
        public void Alloc_Normal()
        {
            object o = new object();
            var h = GCHandle.Alloc(o);
            Assert.True(h.IsAllocated);
            Assert.Equal(o, h.Target);
            h.Free();
        }

        [UnitTest]
        public unsafe void Alloc_Pinned_ByteArray()
        {
            byte[] data = new byte[] {1,2,3,4,5};
            var h = GCHandle.Alloc(data, GCHandleType.Pinned);
            IntPtr addr = h.AddrOfPinnedObject();
            Assert.True(addr != IntPtr.Zero);
            Assert.Equal(1, ((byte*)addr)[0]);
            h.Free();
        }

        [UnitTest]
        public void Alloc_Weak()
        {
            object o = new object();
            var h = GCHandle.Alloc(o, GCHandleType.Weak);
            Assert.True(h.IsAllocated);
            Assert.Equal(o, h.Target);
            h.Free();
        }
    }
}
