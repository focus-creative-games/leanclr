using System;
using System.Globalization;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Globalization_CultureData : TestCaseBase
    {
        [UnitTest]
        public void FillNumberData_ReturnsRightValue()
        {
            CultureInfo enUs = CultureInfo.GetCultureInfo("en-US");
            NumberFormatInfo enUsNfi = enUs.NumberFormat;
            Assert.NotNull(enUsNfi);
            Assert.Equal("$", enUsNfi.CurrencySymbol);
            Assert.Equal(2, enUsNfi.NumberDecimalDigits);
            Assert.Equal(".", enUsNfi.NumberDecimalSeparator);
            Assert.Equal(",", enUsNfi.NumberGroupSeparator);
            Assert.Equal("-", enUsNfi.NegativeSign);
            Assert.Equal("+", enUsNfi.PositiveSign);

            CultureInfo zhCn = CultureInfo.GetCultureInfo("zh-CN");
            NumberFormatInfo zhCnNfi = zhCn.NumberFormat;
            Assert.NotNull(zhCnNfi);
            Assert.IsTrue(zhCnNfi.CurrencySymbol == "\u00a5" || zhCnNfi.CurrencySymbol == "\uffe5");
            Assert.Equal(2, zhCnNfi.NumberDecimalDigits);
            Assert.Equal(".", zhCnNfi.NumberDecimalSeparator);
            Assert.Equal(",", zhCnNfi.NumberGroupSeparator);
        }

        [UnitTest]
        public void FillCultureData_ReturnsRightValue()
        {
            // GetCultureInfo loads CultureData and invokes fill_culture_data(datetime_index) via icall.
            CultureInfo culture = CultureInfo.GetCultureInfo("zh-CN");
            Assert.NotNull(culture);
            Assert.Equal("zh-CN", culture.Name);
            Assert.Equal(0x0804, culture.LCID);

            DateTimeFormatInfo dtfi = culture.DateTimeFormat;
            Assert.NotNull(dtfi);

            // Values from fill_culture_data (s_datetime_zh) for zh-CN datetime_index 187.
            Assert.Equal("\u4e0a\u5348", dtfi.AMDesignator);
            Assert.Equal("\u4e0b\u5348", dtfi.PMDesignator);
            Assert.Equal(":", dtfi.TimeSeparator);
            Assert.IsTrue((int)dtfi.FirstDayOfWeek >= 0 && (int)dtfi.FirstDayOfWeek <= 6);
            Assert.Equal((int)CalendarWeekRule.FirstDay, (int)dtfi.CalendarWeekRule);

            Assert.IsTrue(dtfi.GetAllDateTimePatterns('T').Length > 0);
            Assert.IsTrue(dtfi.GetAllDateTimePatterns('t').Length > 0);
        }

        private static void AssertStringArrayEqual(string[] expected, string[] actual)
        {
            Assert.NotNull(actual);
            Assert.Equal(expected.Length, actual.Length);
            for (int i = 0; i < expected.Length; i++)
            {
                Assert.Equal(expected[i], actual[i]);
            }
        }
    }
}
