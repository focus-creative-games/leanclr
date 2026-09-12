namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunBranchInstructions()
        {
            RunBranchBr();
            RunBranchBrtrue();
            RunBranchBrfalse();
            RunBranchBeq();
            RunBranchBneUn();
            RunBranchRelational();
            RunBranchSwitch();
        }

        public static void RunBranchRelational()
        {
            RunBranchBge();
            RunBranchBgeUn();
            RunBranchBgt();
            RunBranchBgtUn();
            RunBranchBle();
            RunBranchBleUn();
            RunBranchBlt();
            RunBranchBltUn();
        }

        public static void RunBranchBr()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Br));
        }

        public static void RunBranchBrtrue()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_brtrue));
        }

        public static void RunBranchBrfalse()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Brfalse));
        }

        public static void RunBranchSwitch()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_switch));
        }

        public static void RunBranchBeq()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Beq));
        }

        public static void RunBranchBneUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Bne_un));
        }

        public static void RunBranchBge()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Bge));
        }

        public static void RunBranchBgeUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Bge_un));
        }

        public static void RunBranchBgt()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Bgt));
        }

        public static void RunBranchBgtUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Bgt_un));
        }

        public static void RunBranchBle()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Ble));
        }

        public static void RunBranchBleUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Ble_un));
        }

        public static void RunBranchBlt()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Blt));
        }

        public static void RunBranchBltUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Branches.TC_Blt_un));
        }
    }
}
