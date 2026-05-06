using System;
using System.Diagnostics;


internal class TC_Stopwatch
{
    [UnitTest]
    public void ElapsedTicks_MethodExists_AndReturnTypeIsInt64()
    {

        var w = new Stopwatch();
        w.Start();
        w.Stop();
        long value = w.ElapsedTicks;
        Assert.True(value >= 0); // Just check that it returns a non-negative value, which is expected for a timestamp
    }

    [UnitTest]
    public void ElapsedMilliseconds_MethodExists_AndReturnTypeIsInt64()
    {

        var w = new Stopwatch();
        w.Start();
        w.Stop();
        long value = w.ElapsedMilliseconds;
        Assert.True(value >= 0); // Just check that it returns a non-negative value, which is expected for a timestamp
    }
}
