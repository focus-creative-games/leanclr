


using System;

internal class TC_call_aot
{
    public class ForFunClass
    {
        public int x;

        public int Show2()
        {
            return 2;
        }


    }

    [UnitTest]
    public void class_null_this()
    {
        ForFunClass a = null;
        Assert.ExpectException<NullReferenceException>(() =>
        {
            a.Show2();
        });
    }

}

