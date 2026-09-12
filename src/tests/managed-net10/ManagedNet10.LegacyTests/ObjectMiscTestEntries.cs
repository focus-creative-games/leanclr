namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunMiscInstructions()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mics.TC_nop));
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Mics.TC_pop));
        }

        public static void RunObjectInstructions()
        {
            RunObjectCastClass();
            RunObjectCpobj();
            RunObjectInitobj();
            RunObjectIsinst();
            RunObjectLdobj();
            RunObjectLdstr();
            RunObjectLdtoken();
            RunObjectMakerefany();
            RunObjectSizeof();
            RunObjectStobj();
        }

        public static void RunObjectCastClass()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_castclass));
        }

        public static void RunObjectCpobj()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_cpobj));
        }

        public static void RunObjectInitobj()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_initobj));
        }

        public static void RunObjectIsinst()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_isinst));
        }

        public static void RunObjectLdobj()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_ldobj));
        }

        public static void RunObjectLdstr()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_ldstr));
        }

        public static void RunObjectLdtoken()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_ldtoken));
        }

        public static void RunObjectLdtokenMethod1()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Instruments.Objs.TC_ldtoken), "method_1");
        }

        public static void RunObjectMakerefany()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_makerefany_reftype_refvalue));
        }

        public static void RunObjectSizeof()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_sizeof));
        }

        public static void RunObjectStobj()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Objs.TC_stobj));
        }
    }
}
