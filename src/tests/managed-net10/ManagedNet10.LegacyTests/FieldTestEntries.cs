namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunFieldInstructions()
        {
            RunFieldLoadInstance();
            RunFieldLoadInstanceAddress();
            RunFieldLoadStatic();
            RunFieldStoreInstance();
            RunFieldStoreStatic();
            RunFieldThreadStatic();
            RunFieldStaticConstructor();
        }

        public static void RunFieldLoadInstance()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_ldfld_aot),
                typeof(Tests.Instruments.Fields.TC_ldfld_interp));
        }

        public static void RunFieldLoadInstanceAddress()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_ldflda_aot),
                typeof(Tests.Instruments.Fields.TC_ldflda_interp));
        }

        public static void RunFieldLoadStatic()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_ldsfld_aot),
                typeof(Tests.Instruments.Fields.TC_ldsfld_interp));
        }

        public static void RunFieldStoreInstance()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_stfld_aot),
                typeof(Tests.Instruments.Fields.TC_stfld_interp));
        }

        public static void RunFieldStoreStatic()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_stsfld_aot),
                typeof(Tests.Instruments.Fields.TC_stsfld_interp));
        }

        public static void RunFieldThreadStatic()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Fields.TC_ldsfld_interp_threadstatic),
                typeof(Tests.Instruments.Fields.TC_stsfld_interp_threadstatic));
        }

        public static void RunFieldStaticConstructor()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Fields.TC_stsfld_i2_cctor));
        }
    }
}
