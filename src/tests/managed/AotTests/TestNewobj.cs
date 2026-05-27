
public class TestNewobj
{
    class AotClass
    {
        public int x;
        public AotClass(int x)
        {
            this.x = x;
        }
    }

    struct AotStruct
    {
        public int x;
        public AotStruct(int x)
        {
            this.x = x;
        }
    }

    class ClassInterp
    {
        public int y;
        public ClassInterp(int y)
        {
            this.y = y;
        }
    }

    public struct StructInterp
    {
        public int y;
        public StructInterp(int y)
        {
            this.y = y;
        }
    }


    [UnitTest]
    public void CallClassAot()
    {
        AotClass a = new AotClass(1);
        Assert.Equal(1, a.x);
    }

    [UnitTest]
    public void CallStructAot()
    {
        AotStruct s = new AotStruct(1);
        Assert.Equal(1, s.x);
    }

    [UnitTest]
    public void CallClassInterp()
    {
        ClassInterp a = new ClassInterp(1);
        Assert.Equal(1, a.y);
    }

    [UnitTest]
    public void CallStructInterp()
    {
        StructInterp s = new StructInterp(1);
        Assert.Equal(1, s.y);
    }
}
