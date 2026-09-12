namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunConvertInstructions()
        {
            RunConvertSignedInteger();
            RunConvertUnsignedInteger();
            RunConvertFloatingPoint();
            RunConvertOverflow();
        }

        public static void RunConvertUnsignedInteger()
        {
            RunConvertU();
            RunConvertU1();
            RunConvertU2();
            RunConvertU4();
            RunConvertU8();
        }

        public static void RunConvertU()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_u));
        }

        public static void RunConvertU1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_u1));
        }

        public static void RunConvertU2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_u2));
        }

        public static void RunConvertU4()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Converts.TC_conv_u4));
            LegacyTestRunner.RunType(typeof(ConvertUnsignedNet10Semantics));
        }

        public static void RunConvertU8()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Converts.TC_conv_u8));
            LegacyTestRunner.RunType(typeof(ConvertUnsignedNet10Semantics));
        }

        public static void RunConvertSignedInteger()
        {
            RunConvertI();
            RunConvertI1();
            RunConvertI2();
            RunConvertI4();
            RunConvertI8();
        }

        public static void RunConvertI()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Converts.TC_conv_i));
            LegacyTestRunner.RunType(typeof(ConvertNativeIntNet10Semantics));
        }

        public static void RunConvertI1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_i1));
        }

        public static void RunConvertI2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_i2));
        }

        public static void RunConvertI4()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Converts.TC_conv_i4));
            LegacyTestRunner.RunType(typeof(ConvertInt32Net10Semantics));
        }

        public static void RunConvertI8()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Converts.TC_conv_i8));
            LegacyTestRunner.RunType(typeof(ConvertInt64Net10Semantics));
        }

        public static void RunConvertFloatingPoint()
        {
            RunConvertR4();
            RunConvertR8();
            RunConvertRUn();
        }

        public static void RunConvertR4()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_r4));
        }

        public static void RunConvertR8()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_r8));
        }

        public static void RunConvertRUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_r_un));
        }

        public static void RunConvertNativeInteger()
        {
            RunConvertI();
            RunConvertU();
        }

        public static void RunConvertOverflow()
        {
            RunConvertOverflowI1();
            RunConvertOverflowI2();
            RunConvertOverflowI4();
            RunConvertOverflowI8();
            RunConvertOverflowU1();
            RunConvertOverflowU2();
            RunConvertOverflowU4();
            RunConvertOverflowU8();
            RunConvertOverflowIl();
        }

        public static void RunConvertOverflowI1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_i1));
        }

        public static void RunConvertOverflowI2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_i2));
        }

        public static void RunConvertOverflowI4()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_i4));
        }

        public static void RunConvertOverflowI8()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_i8));
        }

        public static void RunConvertOverflowU1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_u1));
        }

        public static void RunConvertOverflowU2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_u2));
        }

        public static void RunConvertOverflowU4()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_u4));
        }

        public static void RunConvertOverflowU8()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Converts.TC_conv_ovf_u8));
        }

        public static void RunConvertOverflowIl()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instructions.Converts.TC_conv_ovf_il));
        }
    }

    internal sealed class ConvertUnsignedNet10Semantics
    {
        [UnitTest]
        public void ConvU4OverflowFloatReturnsMaxValue()
        {
            float value = 0x100000001;
            uint converted = (uint)value;
            Assert.Equal(uint.MaxValue, converted);
        }

        [UnitTest]
        public void ConvU4OverflowDoubleReturnsMaxValue()
        {
            double value = 0x1_00000001;
            uint converted = (uint)value;
            Assert.Equal(uint.MaxValue, converted);
        }

        [UnitTest]
        public void ConvU4NegativeFloatReturnsZero()
        {
            float value = -1;
            uint converted = (uint)value;
            Assert.Equal(0u, converted);
        }

        [UnitTest]
        public void ConvU4NegativeDoubleReturnsZero()
        {
            double value = -1;
            uint converted = (uint)value;
            Assert.Equal(0u, converted);
        }

        [UnitTest]
        public void ConvU8NegativeFloatReturnsZero()
        {
            float value = -1;
            ulong converted = (ulong)value;
            Assert.Equal(0ul, converted);
        }

        [UnitTest]
        public void ConvU8NegativeDoubleReturnsZero()
        {
            double value = -1;
            ulong converted = (ulong)value;
            Assert.Equal(0ul, converted);
        }

        [UnitTest]
        public void ConvU8OverflowFloatKeepsUInt64Value()
        {
            float value = 0x100000001;
            ulong converted = (ulong)value;
            Assert.Equal(4294967296ul, converted);
        }

        [UnitTest]
        public void ConvU8OverflowDoubleKeepsUInt64Value()
        {
            double value = 0x1_00000001;
            ulong converted = (ulong)value;
            Assert.Equal(4294967297ul, converted);
        }

        [UnitTest]
        public void ConvU8OverflowNegativeFloatReturnsZero()
        {
            float value = -0x100000001;
            ulong converted = (ulong)value;
            Assert.Equal(0ul, converted);
        }

        [UnitTest]
        public void ConvU8OverflowNegativeDoubleReturnsZero()
        {
            double value = -0x1_00000001;
            ulong converted = (ulong)value;
            Assert.Equal(0ul, converted);
        }
    }

    internal sealed class ConvertNativeIntNet10Semantics
    {
        [UnitTest]
        public void ConvINativeFloatOverflowUpKeepsNativeWidth()
        {
            float value = 0x100000001;
            nint converted = (nint)(long)value;
            Assert.Equal(4294967296L, (long)converted);
        }

        [UnitTest]
        public void ConvINativeFloatOverflowDownKeepsNativeWidth()
        {
            float value = -0x100000001;
            nint converted = (nint)(long)value;
            Assert.Equal(-4294967296L, (long)converted);
        }

        [UnitTest]
        public void ConvINativeDoubleOverflowUpKeepsNativeWidth()
        {
            double value = 0x1_00000001;
            nint converted = (nint)(long)value;
            Assert.Equal(4294967297L, (long)converted);
        }

        [UnitTest]
        public void ConvINativeDoubleOverflowDownKeepsNativeWidth()
        {
            double value = -0x1_00000001;
            nint converted = (nint)(long)value;
            Assert.Equal(-4294967297L, (long)converted);
        }
    }

    internal sealed class ConvertInt32Net10Semantics
    {
        [UnitTest]
        public void ConvI4FloatOverflowUpReturnsMinValue()
        {
            float value = 0x100000001;
            int converted = (int)value;
            Assert.Equal(int.MinValue, converted);
        }

        [UnitTest]
        public void ConvI4FloatOverflowDownReturnsMinValue()
        {
            float value = -0x100000001;
            int converted = (int)value;
            Assert.Equal(int.MinValue, converted);
        }

        [UnitTest]
        public void ConvI4DoubleOverflowUpReturnsMinValue()
        {
            double value = 0x1_00000001;
            int converted = (int)value;
            Assert.Equal(int.MinValue, converted);
        }

        [UnitTest]
        public void ConvI4DoubleOverflowDownReturnsMinValue()
        {
            double value = -0x1_00000001;
            int converted = (int)value;
            Assert.Equal(int.MinValue, converted);
        }
    }

    internal sealed class ConvertInt64Net10Semantics
    {
        [UnitTest]
        public void ConvI8FloatOverflowUpKeepsInt64Value()
        {
            float value = 0x100000001;
            long converted = (long)value;
            Assert.Equal(4294967296L, converted);
        }

        [UnitTest]
        public void ConvI8FloatOverflowDownKeepsInt64Value()
        {
            float value = -0x100000001;
            long converted = (long)value;
            Assert.Equal(-4294967296L, converted);
        }

        [UnitTest]
        public void ConvI8DoubleOverflowUpKeepsInt64Value()
        {
            double value = 0x1_00000001;
            long converted = (long)value;
            Assert.Equal(4294967297L, converted);
        }

        [UnitTest]
        public void ConvI8DoubleOverflowDownKeepsInt64Value()
        {
            double value = -0x1_00000001;
            long converted = (long)value;
            Assert.Equal(-4294967297L, converted);
        }
    }
}
