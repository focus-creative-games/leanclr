using System;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCoreRegressions()
        {
            int executed = LegacyTestRunner.RunTypes(
                typeof(Tests.Bugs.ArrayCustomArgumentType),
                typeof(Tests.Bugs.Bug20260129),
                typeof(Tests.Bugs.BugException),
                typeof(Tests.Bugs.Bug2022_7_14.Bug_2022_7_14),
                typeof(Tests.Bugs.Bug_2022_7_11),
                typeof(Tests.Bugs.Bug_2022_9_13),
                typeof(Tests.Bugs.Bug_2022_9_22),
                typeof(Tests.Bugs.Bug_2022_9_26),
                typeof(Tests.Bugs.Bug20220927.Bug_2022_9_27),
                typeof(Tests.Bugs.Bug_decimal),
                typeof(Tests.Bugs.Bug_GenericEnum),
                typeof(Tests.Bugs.Bug_GetParameterName),
                typeof(Tests.Bugs.Bug_Interface),
                typeof(Tests.Bugs.Bug_ValueTypeToString_2022_7_22),
                typeof(CoreTests.Tests.Bugs.CustomDelegate),
                typeof(Tests.Bugs.FieldValueByReflection),
                typeof(Tests.Bugs.FixedArraySize),
                typeof(Tests.Bugs.GenericTypeCompareWithNull),
                typeof(Tests.Bugs.InflateGenericClassVTable),
                typeof(Tests.Bugs.InterfaceExplicitImplements),
                typeof(Tests.Bugs.TC_GenericStruct),
                typeof(Tests.Bugs.LoopCallInvoke),
                typeof(Tests.Bugs.MethodSignatureMatch),
                typeof(Tests.Bugs.OptionalParamBinding_2022_10_26),
                typeof(Tests.Bugs.StaticCctorOrder),
                typeof(Tests.Bugs.TC_CallUnmanagedDelegate),
                typeof(Tests.Bugs.TC_GenericInterfaceOnEnableDispatch),
                typeof(Tests.Bugs.TransformConcurrentQueueFail),
                typeof(Tests.Bugs.TupleBug),
                typeof(Tests.Bugs.UnmanagedAccess),
                typeof(Tests.Bugs.ValueTupleGeneric),
                typeof(Tests.Bugs.ZStringFormat));

            LegacyTestRunner.RunMethod(typeof(Tests.Bugs.GenericVirtualMethodReflection), "CallInterface");
            executed++;
            LegacyTestRunner.RunMethod(typeof(Tests.Bugs.GenericVirtualMethodReflection), "CallVirtual");
            executed++;
            LegacyTestRunner.RunMethod(typeof(Tests.Bugs.GenericVirtualMethodReflection), "CallGenericInterfaceVirtual");
            executed++;
            LegacyTestRunner.RunMethod(typeof(Tests.Bugs.ReadRuntimeHandleFromMemberRef), "GenericMemberRef");
            executed++;

            if (executed == 0)
            {
                throw new InvalidOperationException("No CoreTests Regression UnitTest methods executed.");
            }
        }

        public static void RunCoreRegressionGenericMemberRef()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Bugs.ReadRuntimeHandleFromMemberRef), "GenericMemberRef");
        }
    }
}
