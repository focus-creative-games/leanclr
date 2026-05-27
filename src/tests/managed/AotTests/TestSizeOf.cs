
public class TestSizeOf
{

    struct MyStruct
    {
        public int a;
        public double b;

        public MyStruct(int a, double b)
        {
            this.a = a;
            this.b = b;
        }
    }

    [UnitTest]
    public void TestSizeOfInt()
    {
        Assert.Equal(4, sizeof(int));
    }

    [UnitTest]
    public unsafe void TestSizeOfMyStruct()
    {
        Assert.Equal(16, sizeof(MyStruct));
    }

    [UnitTest]
    public unsafe void SizeOfStructWithSize()
    {
        Assert.Equal(32, sizeof(StructWithExplicitLayout1));
    }

    [UnitTest]
    public unsafe void SizeOfStructWithPack()
    {
        Assert.Equal(10, sizeof(StructWithExplicitLayout2));
    }

    [UnitTest]
    public unsafe void SizeOfStructSequenceWithPack()
    {
        Assert.Equal(12, sizeof(StructWithExplicitLayout3));
    }
}

