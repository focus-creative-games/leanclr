using test;


namespace Tests.Instruments.Cmps
{
    internal class TC_Cgt_un: GeneralTestCaseBase
    {
        [UnitTest]
        public void uint_false()
        {
            uint b = 2;
            uint c = 3;
            Assert.False(b > c);
        }

        [UnitTest]
        public void uint_true()
        {
            uint b = 4;
            uint c = 3;
            Assert.True(b > c);
        }

        [UnitTest]
        public void ulong_false()
        {
            ulong b = 2;
            ulong c = 3;
            Assert.False(b > c);
        }

        [UnitTest]
        public void ulong_true()
        {
            ulong b = 4;
            ulong c = 3;
            Assert.True(b > c);
        }

        [UnitTest]
        public void float_false()
        {
            float b = 2;
            float c = 3;
            Assert.False(b > c);
        }

        [UnitTest]
        public void float_true()
        {
            float b = 4;
            float c = 3;
            Assert.True(b > c);
        }

        [UnitTest]
        public void double_false()
        {
            double b = 2;
            double c = 3;
            Assert.False(b > c);
        }

        [UnitTest]
        public void double_true()
        {
            double b = 4;
            double c = 3;
            Assert.True(b > c);
        }
    }
}
