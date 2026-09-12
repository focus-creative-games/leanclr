namespace ManagedNet10.LegacyTests
{
    internal sealed class CoreBootstrapTests
    {
        [UnitTest]
        public void ReturnInt()
        {
            Assert.Equal(42, BootstrapTests.TestReturnValue.ReturnInt());
        }

        [UnitTest]
        public void ReturnString()
        {
            Assert.Equal("abc", BootstrapTests.TestReturnValue.ReturnString());
        }

        [UnitTest]
        public void ReturnStruct()
        {
            BootstrapTests.TestReturnValue.Vec3 value = BootstrapTests.TestReturnValue.ReturnStruct();
            Assert.Equal(1, value.x);
            Assert.Equal(2, value.y);
            Assert.Equal(3, value.z);
        }

        [UnitTest]
        public void PassInt()
        {
            Assert.Equal(12345, BootstrapTests.TestPassArgument.PassInt(12345));
        }

        [UnitTest]
        public void PassString()
        {
            Assert.Equal("hello", BootstrapTests.TestPassArgument.PassString("hello"));
        }

        [UnitTest]
        public void CallSub()
        {
            Assert.Equal(42, BootstrapTests.TestCallMethod.CallSub());
        }
    }

    internal static partial class Program
    {
        public static void RunCoreBootstrap()
        {
            LegacyTestRunner.RunType(typeof(CoreBootstrapTests));
        }
    }
}
