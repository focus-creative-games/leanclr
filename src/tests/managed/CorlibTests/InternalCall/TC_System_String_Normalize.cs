using System.Text;

namespace CorlibTests.InternalCall
{
    internal class TC_System_String_Normalize : TestCaseBase
    {
        [UnitTest]
        public void String_Normalize_LoadsNormalizationResource()
        {
            string composed = "\u00e9";
            string decomposed = "e\u0301";

            Assert.Equal(composed, decomposed.Normalize(NormalizationForm.FormC));
            Assert.Equal(decomposed, composed.Normalize(NormalizationForm.FormD));
        }
    }
}
