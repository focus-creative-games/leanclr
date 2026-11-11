namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCompareInstructions()
        {
            RunCompareCeq();
            RunCompareCgt();
            RunCompareCgtUn();
            RunCompareClt();
            RunCompareCltUn();
        }

        public static void RunCompareCeq()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Cmps.TC_Ceq));
        }

        public static void RunCompareCgt()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Cmps.TC_Cgt));
        }

        public static void RunCompareCgtUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Cmps.TC_Cgt_un));
        }

        public static void RunCompareClt()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Cmps.TC_Clt));
        }

        public static void RunCompareCltUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Cmps.TC_Clt_un));
        }
    }
}
