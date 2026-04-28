
namespace Tests.Bugs
{
    public interface IXXX
    {
        int Activate();
    }
    public class XXX : IXXX
    {
        int IXXX.Activate()
        {
            return 1;
        }

        protected virtual int Activate()
        {
            return 2;
        }
    }
    public class YYY : XXX, IXXX
    {
        public new int Activate()
        {
            return 3;
        }
    }

    class Bug20260129 : GeneralTestCaseBase
    {
        [UnitTest]
        public void TestExplicitInterfaceImplementationWithSameName()
        {
            IXXX obj = new YYY();
            int result = obj.Activate();
            Assert.Equal(3, result);
        }
    }
}
