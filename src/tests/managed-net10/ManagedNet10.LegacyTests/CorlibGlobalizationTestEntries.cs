namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibGlobalization()
        {
            RunCorlibCultureInfo();
            RunCorlibCultureData();
            RunCorlibGlobalizationNet10Semantics();
        }

        public static void RunCorlibCultureInfo()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Globalization_CultureInfo));
        }

        public static void RunCorlibCultureData()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Globalization_CultureData));
        }

        public static void RunCorlibGlobalizationNet10Semantics()
        {
            LegacyTestRunner.RunType(typeof(CorlibGlobalizationNet10Semantics));
        }
    }

    internal sealed class CorlibGlobalizationNet10Semantics
    {
        [UnitTest]
        public void NumberFormatUsesCurrentNet10CultureData()
        {
            AssertNet10NumberFormat("en-US", "$");
            AssertNet10NumberFormat("zh-CN", null);
        }

        private static void AssertNet10NumberFormat(string cultureName, string expectedCurrencySymbol)
        {
            System.Globalization.NumberFormatInfo numberFormat =
                System.Globalization.CultureInfo.GetCultureInfo(cultureName).NumberFormat;

            Assert.NotNull(numberFormat);
            Assert.Equal(3, numberFormat.NumberDecimalDigits);
            Assert.Equal(".", numberFormat.NumberDecimalSeparator);
            Assert.Equal(",", numberFormat.NumberGroupSeparator);
            Assert.Equal("-", numberFormat.NegativeSign);
            Assert.Equal("+", numberFormat.PositiveSign);

            if (expectedCurrencySymbol != null)
            {
                Assert.Equal(expectedCurrencySymbol, numberFormat.CurrencySymbol);
            }
        }
    }
}
