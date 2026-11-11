namespace ManagedNet10.LegacyTests
{
    internal sealed class CoreAppDirectTests
    {
        [UnitTest]
        public void BranchesAndScalars()
        {
            Assert.Equal(7, CoreTests.App.Run(3, 4));
            Assert.Equal("Hello, World!", CoreTests.App.Test1());
            Assert.Equal(2, CoreTests.App.Test2());
            Assert.Equal(2, CoreTests.App.Test3());
            Assert.Equal(1, CoreTests.App.Test4(1, 2));
            Assert.Equal(2, CoreTests.App.Test4(2, 1));
            Assert.Equal(1, CoreTests.App.Test5(1));
            Assert.Equal(2, CoreTests.App.Test5(2));
            Assert.Equal(3, CoreTests.App.Test5(3));
            Assert.Equal(123, CoreTests.App.Test6(123));
            Assert.Equal(10, CoreTests.App.Test7(123));
            Assert.Equal(2, CoreTests.App.Test8(258));
            Assert.Equal(1, CoreTests.App.Test9(1, 2));
            Assert.Equal(-2, CoreTests.App.Test9(2, 1));
        }

        [UnitTest]
        public void BoxingAndArrays()
        {
            Assert.Equal(1, CoreTests.App.Test10(123));
            Assert.Equal(2, CoreTests.App.Test10("abc"));
            Assert.Equal(1, CoreTests.App.Test11("abc"));
            Assert.Equal(2, CoreTests.App.Test11(123));
            Assert.Equal(123, (int)CoreTests.App.Test12(123));
            Assert.Equal(123, CoreTests.App.Test13(123));
            Assert.Equal(5, CoreTests.App.Test20(5).Length);

            int[] values = { 1, 2, 3 };
            Assert.Equal(3, CoreTests.App.Test21(values));
            CoreTests.App.Test22(values);
            Assert.Equal(10, values[0]);
            Assert.Equal(10, CoreTests.App.Test23(values));
            CoreTests.App.Test24(values, 42);
            Assert.Equal(42, values[0]);
        }

        [UnitTest]
        public void TypedRefsAndStackalloc()
        {
            Assert.Equal(10, CoreTests.App.Test30());
            Assert.Equal(10, CoreTests.App.Test40());
        }

        [UnitTest]
        public void InstanceAndStaticFields()
        {
            var value = new CoreTests.A { a = 1, b = 2, c = 3 };
            Assert.Equal(2, CoreTests.App.Test50(value));
            CoreTests.App.Test51(value, 20);
            Assert.Equal(20, value.b);
            CoreTests.App.Test52(value, 30);
            Assert.Equal(30, value.b);

            CoreTests.App.Test54(40);
            Assert.Equal(40, CoreTests.App.Test53());
            CoreTests.App.Test55(50);
            Assert.Equal(50, CoreTests.App.Test53());
        }

        [UnitTest]
        public void CallsAndVirtualDispatch()
        {
            Assert.Equal(1234, CoreTests.App.Test60(1234));
            Assert.Equal(2, CoreTests.App.Test61());
        }

        [UnitTest]
        public void ExceptionControlFlow()
        {
            Assert.Equal(11, CoreTests.App.Test70());
            Assert.Equal(1111, CoreTests.App.Test71());
            Assert.Equal(1131, CoreTests.App.Test72());
            Assert.Equal(10111, CoreTests.App.Test73());
            Assert.Equal(1111, CoreTests.App.Test74());
            Assert.Equal(11111, CoreTests.App.Test75());
            Assert.Equal(1111, CoreTests.App.Test76());
            Assert.Equal(2, CoreTests.App.Test77());
            Assert.Equal(2, CoreTests.App.Test78());
            Assert.Equal(3, CoreTests.App.Test79());
        }
    }

    internal static partial class Program
    {
        public static void RunCoreAppDirect()
        {
            LegacyTestRunner.RunType(typeof(CoreAppDirectTests));
        }
    }
}
