
class TC_conv_r_un
{
    [UnitTest]
    public void uint_1()
    {
        uint x = (uint)(object)1U;
        float y = x;
        Assert.Equal(1f, y);
    }


    [UnitTest]
    public void ulong_1()
    {
        ulong x = (ulong)(object)1UL;
        float y = x;
        Assert.Equal(1f, y);
    }
}
