
public class TestSwitch
{
    [UnitTest]

    public void seq_1()
    {
        int a = 2;
        int b;
        switch (a)
        {
        case 0:
            b = 1;
            break;
        case 1:
            b = 2;
            break;
        case 2:
            b = 3;
            break;
        default:
            b = 4;
            break;
        }
        Assert.Equal(3, b);
    }
}

