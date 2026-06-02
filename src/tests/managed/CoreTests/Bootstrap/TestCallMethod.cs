namespace BootstrapTests
{
    class TestCallMethod
    {
        public static int CallSub()
        {
            return Sub();
        }

        public static int Sub()
        {
            return 42;
        }
    }
}
