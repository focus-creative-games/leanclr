
using Tests.Fixtures;
using UnityEngine;


namespace Tests
{
    public class WebGLMethodBridge : TestCaseBase
    {

        [UnitTest]
        public void ColorToColor32_ImplicitConversion()
        {
            Color32 c = new Color(1f, 1f, 1f, 1f);
            Assert.Equal(255f, c.r);
        }


        [UnitTest]
        public void ColorToColor32_Ctor()
        {
           Color32 c = new Color32(new Color(1f, 1f, 1f, 1f));
        }
    }
}
