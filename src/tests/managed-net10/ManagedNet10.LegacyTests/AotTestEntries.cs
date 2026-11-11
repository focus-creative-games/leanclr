namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunAotCallInterp()
        {
            LegacyTestRunner.RunType(typeof(TestCall));
        }

        public static void RunAotCallVirInterp()
        {
            LegacyTestRunner.RunType(typeof(TestCallVir));
        }

        public static void RunAotCallInterop()
        {
            RunAotCallInterp();
            RunAotCallVirInterp();
        }

        public static void RunAotMisc()
        {
            LegacyTestRunner.RunTypes(
                typeof(TestNewMdArray),
                typeof(TestLdslfda),
                typeof(Tests.CSharp.TestCCtor),
                typeof(TestEvalStackNotEmpty),
                typeof(TC_Stopwatch),
                typeof(TC_MonoPInvokeCallback));
        }
    }
}
