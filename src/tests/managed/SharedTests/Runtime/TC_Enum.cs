using System;

namespace Tests.CSharp
{
    internal enum MyColor
    {
        red,
        green,
        blue = 8,
        yellow,
    }
    internal class TC_Enum : GeneralTestCaseBase
    {
        enum Color
        {
            red,
            green,
            blue = 8,
            yellow,
        }

        [UnitTest]
        public void GetRed_0()
        {
            Assert.Equal(0, (int)Color.red);
        }

        [UnitTest]
        public void GetGreen_1()
        {
            Assert.Equal(1, (int)Color.green);
        }

        [UnitTest]
        public void GetRedAdd_2()
        {
            Assert.Equal(2, (int)(Color.red + 2));
        }

        [UnitTest]
        public void GetGreenAdd_3()
        {
            Assert.Equal(3, (int)(Color.green + 2));
        }

        [UnitTest]
        public void GetBlue_8()
        {
            Assert.Equal(8, (int)Color.blue);
        }

        [UnitTest]
        public void GetYellor_9()
        {
            Assert.Equal(9, (int)Color.yellow);
        }

        enum ByteEnum : byte
        {
            Red,
            Green = 1,
        }

        [UnitTest]
        public void TestByteEnum()
        {
            int h = ByteEnum.Green.GetHashCode();
            Assert.Equal(1, h);
        }

        enum SByteEnum : sbyte
        {
            Red = -1,
            Green = 1,
        }

#if IL2CPP_ONLY
        [UnitTest]
        public void TestSByteEnum()
        {
#if !UNITY_EDITOR
            int h = SByteEnum.Green.GetHashCode();
            Assert.Equal(1, h);
            int h2 = SByteEnum.Red.GetHashCode();
            Assert.Equal(-1, h2);
#endif
        }

        enum ShortEnum : short
        {
            Red = -1,
            Green = 1,
        }

        [UnitTest]
        public void TestShortEnum()
        {
#if !UNITY_EDITOR
            int h = ShortEnum.Green.GetHashCode();
            Assert.Equal(1, h);
            int h2 = ShortEnum.Red.GetHashCode();
            Assert.Equal(65535, h2);
#endif
        }
#endif

        enum UShortEnum : ushort
        {
            Red = 1,
            Green = 2,
        }

        [UnitTest]
        public void TestUShortEnum()
        {
            int h = UShortEnum.Green.GetHashCode();
            Assert.Equal(2, h);
            int h2 = UShortEnum.Red.GetHashCode();
            Assert.Equal(1, h2);
        }

        enum IntEnum : int
        {
            Red = -1,
            Green = 1,
        }

        [UnitTest]
        public void TestIntEnum()
        {
            int h = IntEnum.Green.GetHashCode();
            Assert.Equal(1, h);
            int h2 = IntEnum.Red.GetHashCode();
            Assert.Equal(-1, h2);
        }

        enum UIntEnum : uint
        {
            Red = 1,
            Green = 2,
        }

        [UnitTest]
        public void TestUIntEnum()
        {
            int h = UIntEnum.Green.GetHashCode();
            Assert.Equal(2, h);
            int h2 = UIntEnum.Red.GetHashCode();
            Assert.Equal(1, h2);
        }

        enum LongEnum : long
        {
            Red = -1,
            Green = 1,
        }

        [UnitTest]
        public void TestLongEnum()
        {
#if !UNITY_EDITOR
            int h = LongEnum.Green.GetHashCode();
            Assert.Equal(1, h);
            int h2 = LongEnum.Red.GetHashCode();
            Assert.Equal(0, h2);
#endif
        }

        enum ULongEnum : ulong
        {
            Red = 1,
            Green = 2,
        }

        [UnitTest]
        public void TestULongEnum()
        {
            int h = ULongEnum.Green.GetHashCode();
            Assert.Equal(2, h);
            int h2 = ULongEnum.Red.GetHashCode();
            Assert.Equal(1, h2);
        }


        private int AddEnum<T>(T x, int c)
        {
            return x.GetHashCode() / c;
        }

        [UnitTest]
        public void TestIntEnumStack()
        {
            int v = AddEnum(IntEnum.Red, 2);
            Assert.Equal(0, v);
        }

        [UnitTest]
        public void TestLongEnumStack()
        {
            int v = AddEnum(LongEnum.Green, 2);
            Assert.Equal(0, v);
        }
    }
}
