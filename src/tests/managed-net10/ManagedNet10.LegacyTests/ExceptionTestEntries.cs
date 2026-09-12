namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        private const int FatEhClauseCount = 12;

        public static void RunExceptionInstructions()
        {
            RunExceptionEndFilter();
            RunExceptionEndFinally();
            RunExceptionFatEhSection();
            RunExceptionLeave();
            RunExceptionRethrow();
            RunExceptionThrow();
        }

        public static void RunExceptionEndFilter()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_endfilter));
        }

        public static void RunExceptionEndFinally()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_endfinally));
        }

        public static void RunExceptionFatEhSection()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_fat_eh_section));
        }

        public static void RunExceptionFatEhExecuteOnly()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Instruments.Exceptions.TC_fat_eh_section), "run_method_with_fat_eh_section");
        }

        public static void RunExceptionFatEhReflectionOnly()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Instruments.Exceptions.TC_fat_eh_section), "reflection_reads_all_exception_clauses");
        }

        public static void RunExceptionFatEhReflectionDirect()
        {
            System.Reflection.MethodInfo method = typeof(Tests.Instruments.Exceptions.TC_fat_eh_section).GetMethod(
                nameof(Tests.Instruments.Exceptions.TC_fat_eh_section.MethodWithManyCatchClauses),
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            Assert.NotNull(method);

            System.Reflection.MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
            Assert.Equal(FatEhClauseCount, body.ExceptionHandlingClauses.Count);
        }

        public static void RunExceptionFatEhClauseCountOnly()
        {
            System.Reflection.MethodInfo method = typeof(Tests.Instruments.Exceptions.TC_fat_eh_section).GetMethod(
                nameof(Tests.Instruments.Exceptions.TC_fat_eh_section.MethodWithManyCatchClauses),
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            Assert.NotNull(method);

            System.Reflection.MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);

            int count = body.ExceptionHandlingClauses.Count;
            if (count != FatEhClauseCount)
            {
                throw new System.Exception("Unexpected exception handling clause count.");
            }
        }

        public static void RunExceptionFatEhLookupOnly()
        {
            System.Reflection.MethodInfo method = typeof(Tests.Instruments.Exceptions.TC_fat_eh_section).GetMethod(
                nameof(Tests.Instruments.Exceptions.TC_fat_eh_section.MethodWithManyCatchClauses),
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            Assert.NotNull(method);
        }

        public static void RunExceptionFatEhGetBodyOnly()
        {
            System.Reflection.MethodInfo method = typeof(Tests.Instruments.Exceptions.TC_fat_eh_section).GetMethod(
                nameof(Tests.Instruments.Exceptions.TC_fat_eh_section.MethodWithManyCatchClauses),
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            Assert.NotNull(method);

            System.Reflection.MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
        }

        public static void RunExceptionFatEhClauseCollectionOnly()
        {
            System.Reflection.MethodInfo method = typeof(Tests.Instruments.Exceptions.TC_fat_eh_section).GetMethod(
                nameof(Tests.Instruments.Exceptions.TC_fat_eh_section.MethodWithManyCatchClauses),
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);
            Assert.NotNull(method);

            System.Reflection.MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
            Assert.NotNull(body.ExceptionHandlingClauses);
        }

        public static void RunExceptionLeave()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_leave));
        }

        public static void RunExceptionRethrow()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_rethrow));
        }

        public static void RunExceptionThrow()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Exceptions.TC_throw));
        }

        public static void RunExceptionThrowFinallyCatch()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Instruments.Exceptions.TC_throw), "finally_catch");
        }
    }
}
