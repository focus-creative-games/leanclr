using System;
using System.Globalization;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Globalization_CultureInfo : TestCaseBase
    {
        [CoversIcall("System.Globalization.CultureInfo::construct_internal_locale_from_name(System.String)")]
        [UnitTest]
        public void CreateSpecificCulture_FromName()
        {
            CultureInfo culture = new CultureInfo("en-US");
            Assert.Equal("en-US", culture.Name);
        }

        [CoversIcall("System.Globalization.CultureInfo::construct_internal_locale_from_lcid(System.Int32)")]
        [UnitTest]
        public void CreateSpecificCulture_FromLcid()
        {
            CultureInfo culture = new CultureInfo(1033);
            Assert.IsTrue(culture.Name.StartsWith("en"));
        }

        [CoversIcall("System.Globalization.CultureInfo::get_current_locale_name")]
        [UnitTest]
        public void CurrentCulture_HasName()
        {
            string name = CultureInfo.CurrentCulture.Name;
            Assert.NotNull(name);
            Assert.IsTrue(name.Length > 0);
        }

        [CoversIcall("System.Globalization.CultureInfo::internal_get_cultures(System.Boolean,System.Boolean,System.Boolean)")]
        [UnitTest]
        public void GetCultures_ReturnsNeutralAndSpecific()
        {
            CultureInfo[] cultures = CultureInfo.GetCultures(CultureTypes.AllCultures);
            Assert.IsTrue(cultures.Length > 0);
        }
    }
}
