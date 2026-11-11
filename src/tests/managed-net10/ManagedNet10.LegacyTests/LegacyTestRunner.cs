using System;
using System.Reflection;

namespace ManagedNet10.LegacyTests
{
    internal static class LegacyTestRunner
    {
        private const BindingFlags TestMethodFlags =
            BindingFlags.Public |
            BindingFlags.NonPublic |
            BindingFlags.Instance |
            BindingFlags.Static;

        public static int RunTypes(params Type[] types)
        {
            int executed = 0;
            for (int i = 0; i < types.Length; i++)
            {
                executed += RunType(types[i]);
            }

            return executed;
        }

        public static int RunAssembly(Assembly assembly)
        {
            int executed = 0;
            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to enumerate legacy test types in " + assembly.FullName, ex);
            }

            for (int i = 0; i < types.Length; i++)
            {
                executed += RunType(types[i], skipNet10ReplacedLegacyTests: true);
            }

            if (executed == 0)
            {
                throw new Exception("No UnitTest methods executed for " + assembly.FullName);
            }

            return executed;
        }

        public static int RunAssemblyPrefix(Assembly assembly, int maxTypes)
        {
            int executed = 0;
            Type[] types = assembly.GetTypes();
            int count = maxTypes < types.Length ? maxTypes : types.Length;
            for (int i = 0; i < count; i++)
            {
                executed += RunType(types[i], skipNet10ReplacedLegacyTests: true);
            }

            return executed;
        }

        public static int RunType(Type type)
        {
            return RunType(type, skipNet10ReplacedLegacyTests: false);
        }

        public static int RunTypeWithNet10Replacements(Type type)
        {
            return RunType(type, skipNet10ReplacedLegacyTests: true);
        }

        private static int RunType(Type type, bool skipNet10ReplacedLegacyTests)
        {
            bool trace = Environment.GetEnvironmentVariable("LEANCLR_LEGACY_TRACE") == "1";
            if (trace)
            {
                Console.WriteLine("legacy-type: " + type.FullName);
            }

            bool isIgnored;
            try
            {
                isIgnored = Attribute.IsDefined(type, typeof(IgnoreTestAttribute), inherit: true);
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to inspect legacy test attributes on type " + type.FullName, ex);
            }

            if (isIgnored)
            {
                return 0;
            }

            object instance = null;
            MethodInfo[] methods;
            try
            {
                methods = type.GetMethods(TestMethodFlags);
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to enumerate legacy test methods on " + type.FullName, ex);
            }

            int executed = 0;

            for (int i = 0; i < methods.Length; i++)
            {
                MethodInfo method = methods[i];
                bool isUnitTest;
                try
                {
                    isUnitTest = Attribute.IsDefined(method, typeof(UnitTestAttribute), inherit: true);
                }
                catch (Exception ex)
                {
                    throw new Exception("Failed to inspect legacy test attributes on " + type.FullName + "." + method.Name, ex);
                }

                if (!isUnitTest)
                {
                    continue;
                }
                if (skipNet10ReplacedLegacyTests && IsNet10ReplacedLegacyTest(type, method.Name))
                {
                    continue;
                }
                if (method.ReturnType != typeof(void) || method.GetParameters().Length != 0)
                {
                    continue;
                }

                if (trace)
                {
                    Console.WriteLine("legacy-test: " + type.FullName + "." + method.Name);
                }

                if (!method.IsStatic && instance == null)
                {
                    instance = Activator.CreateInstance(type);
                }

                try
                {
                    TestCaseBase testCase = instance as TestCaseBase;
                    testCase?.SetUp();
                    try
                    {
                        method.Invoke(instance, null);
                    }
                    finally
                    {
                        testCase?.TearDown();
                    }
                    if (trace)
                    {
                        Console.WriteLine("legacy-test-done: " + type.FullName + "." + method.Name);
                    }
                }
                catch (Exception ex)
                {
                    Exception failure = ex.InnerException ?? ex;
                    throw new Exception(
                        "Legacy test failed: " + type.FullName + "." + method.Name + ": " +
                        failure.GetType().FullName + ": " + failure.Message,
                        ex);
                }
                executed++;
            }

            return executed;
        }

        public static void RunMethod(Type type, string methodName)
        {
            bool trace = Environment.GetEnvironmentVariable("LEANCLR_LEGACY_TRACE") == "1";
            if (trace)
            {
                Console.WriteLine("legacy-method-lookup: " + type.FullName + "." + methodName);
            }
            MethodInfo method = type.GetMethod(methodName, TestMethodFlags);
            if (trace)
            {
                Console.WriteLine("legacy-method-found: " + (method == null ? "<null>" : method.Name));
            }
            if (method == null)
            {
                throw new MissingMethodException(type.FullName, methodName);
            }
            if (method.ReturnType != typeof(void) || method.GetParameters().Length != 0)
            {
                throw new InvalidOperationException(type.FullName + "." + methodName + " is not a zero-argument void test method.");
            }

            object instance = method.IsStatic ? null : Activator.CreateInstance(type);
            if (trace)
            {
                Console.WriteLine("legacy-method-invoke: " + type.FullName + "." + method.Name);
            }
            TestCaseBase testCase = instance as TestCaseBase;
            testCase?.SetUp();
            try
            {
                method.Invoke(instance, null);
            }
            finally
            {
                testCase?.TearDown();
            }
        }

        private static bool IsNet10ReplacedLegacyTest(Type type, string methodName)
        {
            // The linked legacy source still carries Mono/mscorlib expectations for this case.
            // RunAssembly uses the net10-specific replacement in CorlibStringNet10Semantics instead.
            if (type == typeof(CorlibTests.InternalCall.TC_System_String) &&
                methodName == "LastIndexOf_EmptyString")
            {
                return true;
            }

            // The original assembly name test was compiled into CorlibTests.dll. The linked
            // net10 assembly intentionally runs under ManagedNet10.LegacyTests instead.
            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_AssemblyName) &&
                methodName == "GetNativeName")
            {
                return true;
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_Assembly))
            {
                return methodName == "LoadAssembly" ||
                    methodName == "LoadAssemblyFullQualifiedName" ||
                    methodName == "Serialization_Call_Assembly_Load_Internal";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_AppDomain) &&
                (methodName == "LoadAssemblyExist_ByName" ||
                methodName == "LoadAssemblyNotExist_ByName"))
            {
                return true;
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Threading_Monitor) &&
                methodName == "WaitPulse_SignalWaitingThread")
            {
                return true;
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeModule) &&
                (methodName == "GetPEKind_ReturnsValidEnums" ||
                methodName == "GetPEKind_ReturnsConsistentValues"))
            {
                return true;
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeAssembly))
            {
                return methodName == "GetFullName" ||
                    methodName == "GetEntryPoint" ||
                    methodName == "GetManifestModule";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeFieldInfo))
            {
                return methodName == "GetRawConstantValue_ForConstField";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Reflection_FieldInfo))
            {
                return methodName == "GetRawConstantValue";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_RuntimeTypeHandle))
            {
                return methodName.StartsWith("HasInstantiation_", StringComparison.Ordinal);
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_RuntimeType))
            {
                return methodName == "GetNestedTypes_Public_ReturnsPublicNestedTypesOnly" ||
                    methodName == "GetNestedTypes_PublicAndNonPublic_ReturnsAllNestedTypes";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Console))
            {
                // The linked legacy source still expects Mono's null-console no-throw behavior.
                // RunAssembly uses CorlibConsoleNet10Semantics for the .NET 10 redirected-input behavior.
                return methodName == "KeyAvailable_ReadWithoutThrow" ||
                    methodName == "TreatControlCAsInput_ToggleWithoutThrow";
            }

            if (type == typeof(CorlibTests.InternalCall.TC_System_Globalization_CultureData) &&
                methodName == "FillNumberData_ReturnsRightValue")
            {
                // Modern .NET culture data reports 3 NumberDecimalDigits for en-US/zh-CN.
                // RunAssembly covers that behavior through CorlibGlobalizationNet10Semantics.
                return true;
            }

            if (type == typeof(Tests.Instruments.Converts.TC_conv_u4))
            {
                return methodName == "float_overflow_up" ||
                    methodName == "float_overflow_down" ||
                    methodName == "double_overflow_up" ||
                    methodName == "double_overflow_down";
            }

            if (type == typeof(Tests.Instruments.Converts.TC_conv_u8))
            {
                return methodName == "float_2" ||
                    methodName == "float_overflow_up" ||
                    methodName == "float_overflow_down" ||
                    methodName == "double_2" ||
                    methodName == "double_overflow_up" ||
                    methodName == "double_overflow_down";
            }

            if (type == typeof(Tests.Instruments.Converts.TC_conv_i))
            {
                return methodName == "float_overflow_up" ||
                    methodName == "float_overflow_down" ||
                    methodName == "double_overflow_up" ||
                    methodName == "double_overflow_down";
            }

            if (type == typeof(Tests.Instruments.Converts.TC_conv_i4))
            {
                return methodName == "float_overflow_up" ||
                    methodName == "float_overflow_down" ||
                    methodName == "double_overflow_up" ||
                    methodName == "double_overflow_down";
            }

            if (type == typeof(Tests.Instruments.Converts.TC_conv_i8))
            {
                return methodName == "float_overflow_up" ||
                    methodName == "float_overflow_down" ||
                    methodName == "double_overflow_up" ||
                    methodName == "double_overflow_down";
            }

            if (type == typeof(Tests.Instruments.Arrays.TC_ldelema))
            {
                return methodName == "UncheckedForStructArray2";
            }

            return false;
        }
    }
}
