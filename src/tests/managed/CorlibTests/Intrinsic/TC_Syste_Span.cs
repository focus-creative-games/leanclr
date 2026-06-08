using System;

namespace Tests.Intrinsic
{
    internal class TC_Syste_Span : TestCaseBase
    {
        [UnitTest]
        public unsafe void GetItem()
        {
            byte[] spanBytes = new byte[2];
            Span<byte> span = spanBytes;
            span[0] = 145;
            span[1] = 99;
            Assert.Equal(145, spanBytes[0]);
            Assert.Equal(99, spanBytes[1]);
        }
    }
}
