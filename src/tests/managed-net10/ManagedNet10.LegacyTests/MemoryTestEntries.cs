namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunMemoryInstructions()
        {
            RunMemoryCpblk();
            RunMemoryDup();
            RunMemoryInitblk();
            RunMemoryLdargIn();
            RunMemoryLdargNormal();
            RunMemoryLdargOut();
            RunMemoryLdargRef();
            RunMemoryLdarga();
            RunMemoryLdc();
            RunMemoryLdftn();
            RunMemoryLdind();
            RunMemoryLdnull();
            RunMemoryLdloc();
            RunMemoryLdloca();
            RunMemoryLocalloc();
            RunMemoryRet();
            RunMemoryStarg();
            RunMemoryStind();
            RunMemoryStloc();
        }

        public static void RunMemoryCpblk()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_cpblk));
        }

        public static void RunMemoryDup()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_dup));
        }

        public static void RunMemoryInitblk()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_initblk));
        }

        public static void RunMemoryLdargIn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldarg_in));
        }

        public static void RunMemoryLdargNormal()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldarg_normal));
        }

        public static void RunMemoryLdargOut()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldarg_out));
        }

        public static void RunMemoryLdargRef()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldarg_ref));
        }

        public static void RunMemoryLdarga()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldarga));
        }

        public static void RunMemoryLdc()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldc));
        }

        public static void RunMemoryLdftn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldftn));
        }

        public static void RunMemoryLdind()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldind));
        }

        public static void RunMemoryLdnull()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldnull));
        }

        public static void RunMemoryLdloc()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldloc));
        }

        public static void RunMemoryLdloca()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ldloca));
        }

        public static void RunMemoryLocalloc()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_localloc));
        }

        public static void RunMemoryRet()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_ret));
        }

        public static void RunMemoryStarg()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_starg));
        }

        public static void RunMemoryStind()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_stind));
        }

        public static void RunMemoryStloc()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mems.TC_stloc));
        }
    }
}
