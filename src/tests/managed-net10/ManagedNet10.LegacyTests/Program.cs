namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        private static void Main()
        {
            RunAll();
        }

        public static void RunAll()
        {
            LegacyTestRunner.RunAssembly(typeof(Program).Assembly);
        }

        public static void RunAllPrefixThenRuntimeType()
        {
            string prefixCountText = System.Environment.GetEnvironmentVariable("LEANCLR_PREFIX_TYPE_COUNT");
            int prefixCount = prefixCountText == null ? 0 : int.Parse(prefixCountText);
            LegacyTestRunner.RunAssemblyPrefix(typeof(Program).Assembly, prefixCount);
            RunCorlibRuntimeTypeLegacy();
        }

        public static void RunAllPrefixThenRuntimeTypeMethod()
        {
            string prefixCountText = System.Environment.GetEnvironmentVariable("LEANCLR_PREFIX_TYPE_COUNT");
            int prefixCount = prefixCountText == null ? 0 : int.Parse(prefixCountText);
            string methodName = System.Environment.GetEnvironmentVariable("LEANCLR_RUNTIME_TYPE_METHOD");
            LegacyTestRunner.RunAssemblyPrefix(typeof(Program).Assembly, prefixCount);
            if (System.Environment.GetEnvironmentVariable("LEANCLR_LEGACY_TRACE") == "1")
            {
                System.Console.WriteLine("legacy-diagnostic: after-prefix method=" + methodName);
            }
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_RuntimeType), methodName);
        }

        public static void RunGcFinalizerMethodThenRuntimeType()
        {
            string methodName = System.Environment.GetEnvironmentVariable("LEANCLR_FINALIZER_METHOD");
            LegacyTestRunner.RunMethod(typeof(GcTests.Finalizer.TC_GC_Finalizer), methodName);
            RunCorlibRuntimeTypeLegacy();
        }

        public static void RunCollectThenRuntimeType()
        {
            System.GC.Collect();
            RunCorlibRuntimeTypeLegacy();
        }

        public static void RunCollectThenSimpleTypeEquality()
        {
            System.GC.Collect();
            Assert.Equal(typeof(string), typeof(string));
            Assert.Equal(typeof(string), typeof(string).MakeArrayType().GetElementType());
        }

        public static void RunLegacyDiscoverySmoke()
        {
            System.Type targetType = typeof(Tests.Instruments.Ariths.TC_sub);
            System.Type[] assemblyTypes = typeof(Program).Assembly.GetTypes();
            bool foundType = false;
            for (int i = 0; i < assemblyTypes.Length; i++)
            {
                if (assemblyTypes[i] == targetType)
                {
                    foundType = true;
                    break;
                }
            }

            if (!foundType)
            {
                Assert.Fail("Assembly.GetTypes did not return TC_sub; count=" + assemblyTypes.Length);
            }
            if (System.Attribute.IsDefined(targetType, typeof(IgnoreTestAttribute), inherit: true))
            {
                Assert.Fail("Attribute.IsDefined incorrectly reported IgnoreTestAttribute on TC_sub");
            }

            System.Reflection.BindingFlags flags =
                System.Reflection.BindingFlags.Public |
                System.Reflection.BindingFlags.NonPublic |
                System.Reflection.BindingFlags.Instance |
                System.Reflection.BindingFlags.Static;
            System.Reflection.MethodInfo[] methods = targetType.GetMethods(flags);
            System.Reflection.MethodInfo selectedMethod = null;
            bool foundMethod = false;
            bool foundUnitTestAttribute = false;
            for (int i = 0; i < methods.Length; i++)
            {
                if (methods[i].Name == "int_vv_1")
                {
                    selectedMethod = methods[i];
                    foundMethod = true;
                    foundUnitTestAttribute = System.Attribute.IsDefined(methods[i], typeof(UnitTestAttribute), inherit: true);
                    break;
                }
            }

            if (!foundMethod)
            {
                Assert.Fail("TC_sub.GetMethods did not return int_vv_1; count=" + methods.Length);
            }
            if (!foundUnitTestAttribute)
            {
                Assert.Fail("Attribute.IsDefined did not see UnitTestAttribute on TC_sub.int_vv_1");
            }
            System.Type returnType = selectedMethod.ReturnType;
            System.Type expectedReturnType = typeof(void);
            if (returnType != expectedReturnType)
            {
                Assert.Fail(
                    "TC_sub.int_vv_1 ReturnType is not System.Void: " + returnType.FullName +
                    "; referenceEquals=" + object.ReferenceEquals(returnType, expectedReturnType) +
                    "; equals=" + returnType.Equals(expectedReturnType) +
                    "; handleEquals=" + returnType.TypeHandle.Equals(expectedReturnType.TypeHandle));
            }
            if (selectedMethod.GetParameters().Length != 0)
            {
                Assert.Fail("TC_sub.int_vv_1 unexpectedly has parameters: " + selectedMethod.GetParameters().Length);
            }
            if (!selectedMethod.IsStatic)
            {
                Assert.Fail("TC_sub.int_vv_1 should be static.");
            }

            int executed = LegacyTestRunner.RunType(targetType);
            if (executed <= 0)
            {
                Assert.Fail("LegacyTestRunner.RunType(TC_sub) did not execute any UnitTest methods.");
            }
        }

        public static void RunActivator()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Activator));
        }

        public static void ActivatorCreateInstanceClass()
        {
            RunActivator();
        }

        public static void ActivatorCreateInstanceStruct()
        {
            RunActivator();
        }

        public static void AOTActivatorCreateInstanceClass()
        {
            RunActivator();
        }

        public static void AOTActivatorCreateInstanceStruct()
        {
            RunActivator();
        }

        public static void TestStackAfterActivatorCreateInstanceClass()
        {
            RunActivator();
        }

        public static void TestStackAfterActivatorCreateInstanceStruct()
        {
            RunActivator();
        }

        public static void NewObjZeroArgument()
        {
            RunActivator();
        }

        public static void CreateInt()
        {
            RunActivator();
        }

        public static void RunArithmeticInstructions()
        {
            RunArithmeticAdd();
            RunArithmeticSub();
            RunArithmeticMul();
            RunArithmeticDivRem();
            RunArithmeticBitwise();
            RunArithmeticNeg();
            RunArithmeticShift();
        }

        public static void RunArithmeticAdd()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_add));
        }

        public static void RunArithmeticAddSmallIntegers()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddIntegers()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddNativeInteger()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddFloatingPoint()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddFloatBasic()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddFloatSpecial()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddDoubleBasic()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddDoubleSpecial()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddDoubleInfinity()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddDoubleNegativeInfinity()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticAddDoubleNaN()
        {
            RunArithmeticAdd();
        }

        public static void RunArithmeticSub()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_sub));
        }

        public static void RunArithmeticMul()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_mul));
        }

        public static void RunArithmeticDivRem()
        {
            RunArithmeticDiv();
            RunArithmeticDivUn();
            RunArithmeticRem();
            RunArithmeticRemUn();
        }

        public static void RunArithmeticDiv()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_div));
        }

        public static void RunArithmeticDivUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_div_un));
        }

        public static void RunArithmeticRem()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_rem));
        }

        public static void RunArithmeticRemUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_un));
        }

        public static void RunArithmeticBitwise()
        {
            RunArithmeticAnd();
            RunArithmeticOr();
            RunArithmeticXor();
            RunArithmeticNot();
        }

        public static void RunArithmeticAnd()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_and));
        }

        public static void RunArithmeticOr()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_or));
        }

        public static void RunArithmeticXor()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_xor));
        }

        public static void RunArithmeticNot()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_not));
        }

        public static void RunArithmeticNeg()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_neg));
        }

        public static void RunArithmeticShift()
        {
            RunArithmeticShl();
            RunArithmeticShr();
            RunArithmeticShrUn();
        }

        public static void RunArithmeticShl()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_shl));
        }

        public static void RunArithmeticShr()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_shr));
        }

        public static void RunArithmeticShrUn()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Ariths.TC_shr_un));
        }
    }
}
