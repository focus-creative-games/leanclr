namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunFunctionLdftnDelegates()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Funcs.TC_ldftn),
                typeof(Tests.Instruments.Funcs.TC_ldvirftn));
        }

        public static void RunFunctionCallBasic()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Funcs.TC_call_aot),
                typeof(Tests.Instruments.Funcs.TC_call_interp));
        }

        public static void RunFunctionCallAot()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_call_aot));
        }

        public static void RunFunctionCallInterp()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_call_interp));
        }

        public static void RunFunctionNewobjConstructors()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Funcs.TC_newobj_aot),
                typeof(Tests.Instruments.Funcs.TC_newobj_interp));
        }

        public static void RunFunctionNewobjAot()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_newobj_aot));
        }

        public static void RunFunctionNewobjInterp()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_newobj_interp));
        }

        public static void RunFunctionCallvirtAot()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_callvir_aot));
        }

        public static void RunFunctionCallvirtInterp()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_callvir_interp));
        }

        public static void RunFunctionAotParamReturn()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Funcs.TC_call_aot_param),
                typeof(Tests.Instruments.Funcs.TC_call_aot_return));
        }

        public static void RunFunctionAotParam()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_call_aot_param));
        }

        public static void RunFunctionAotReturn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_call_aot_return));
        }

        public static void RunFunctionLdftn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_ldftn));
        }

        public static void RunFunctionLdvirtftn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Funcs.TC_ldvirftn));
        }
    }
}
