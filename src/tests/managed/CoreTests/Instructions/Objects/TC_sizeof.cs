
using test;
using System.Runtime.InteropServices;
using AOTDefs;

namespace Tests.Instruments.Objs
{
    [StructLayout(LayoutKind.Explicit, Pack = 8, Size = 919816)]
    public unsafe partial struct CSDT_DESK_PLAYERINFO
    {
        public const int astCampPlayerInfo_length = 32;

        [FieldOffset(0)] internal fixed byte astCampPlayerInfo_bytes[astCampPlayerInfo_length * 28744];
        [FieldOffset(919808)] public int dwPlayerNum;
        [FieldOffset(919812)] public byte bCampNum;
    }

    internal class TC_sizeof : GeneralTestCaseBase
    {

        [UnitTest]
        public unsafe void size_1()
        {
            Assert.Equal(1, sizeof(ValueTypeSize1));
        }


        [UnitTest]
        public unsafe void size_2()
        {
            Assert.Equal(2, sizeof(ValueTypeSize2));
        }

        [UnitTest]
        public unsafe void size_3()
        {
            Assert.Equal(3, sizeof(ValueTypeSize3));
        }

        [UnitTest]
        public unsafe void size_4()
        {
            Assert.Equal(4, sizeof(ValueTypeSize4));
        }

        [UnitTest]
        public unsafe void size_5()
        {
            Assert.Equal(5, sizeof(ValueTypeSize5));
        }

        [UnitTest]
        public unsafe void size_8()
        {
            Assert.Equal(8, sizeof(ValueTypeSize8));
        }

        [UnitTest]
        public unsafe void size_9()
        {
            Assert.Equal(9, sizeof(ValueTypeSize9));
        }

        [UnitTest]
        public unsafe void size_16()
        {
            Assert.Equal(16, sizeof(ValueTypeSize16));
        }

        [UnitTest]
        public unsafe void size_huge()
        {
            var lenval = sizeof(CSDT_DESK_PLAYERINFO);
            Assert.Equal(919816, lenval);
        }
    }
}
