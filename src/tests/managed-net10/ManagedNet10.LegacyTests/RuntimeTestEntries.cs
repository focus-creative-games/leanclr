namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunRuntimeBasics()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.CSharp.TC_String),
                typeof(Tests.CSharp.TC_Enum),
                typeof(Tests.CSharp.TC_Interlocked),
                typeof(Tests.CSharp.TC_using));
        }

        public static void RunRuntimeLanguageFeatures()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.CSharp.TC_foreach),
                typeof(Tests.CSharp.TC_event),
                typeof(Tests.CSharp.TC_Nullable),
                typeof(Tests.CSharp.TC_InterfaceDefaultMethod),
                typeof(Tests.CSharp.TC_Marshal),
                typeof(Tests.CSharp.TC_AOTTypeImplInterpInterface),
                typeof(Tests.CSharp.Dynamics.TC_dynamic),
                typeof(Tests.CSharp.TC_ArrayGenericInterface));
        }

        public static void RunRuntimeDelegates()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.CSharp.Delegates.TC_Delegate_OpenClose),
                typeof(Tests.CSharp.Delegates.TC_Delegate_DynamicInvoke));
        }

        public static void RunRuntimeAsync()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_async));
        }

        public static void RunRuntimeAsyncTestString()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_async), "TestString");
        }

        public static void RunRuntimeAsyncTestInt()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_async), "TestInt");
        }

        public static void RunRuntimeAsyncTestEnum()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_async), "TestEnum");
        }

        public static void RunRuntimeAsyncTestStruct()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_async), "TestStruct");
        }

        public static void RunRuntimeInnerTypeTest2()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Mics.TC_InnerType), "Test2");
        }

        public static void RunRuntimeMethodBridgeDelegateMissing()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Mics.TC_MethodBridge), "DelegateMissing");
        }

        public static void RunRuntimeReflection()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Reflection));
        }

        public static void RunRuntimeCustomAttribute()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.CustomeAttrites.TC_CustomAttribute));
        }

        public static void RunRuntimeReflectionConstString()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_string");
        }

        public static void RunRuntimeReflectionConstByte()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_byte");
        }

        public static void RunRuntimeReflectionConstSByte()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_sbyte");
        }

        public static void RunRuntimeReflectionConstSByte2()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_sbyte2");
        }

        public static void RunRuntimeReflectionConstShort()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_short");
        }

        public static void RunRuntimeReflectionConstShort2()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_short2");
        }

        public static void RunRuntimeReflectionConstInt()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_int");
        }

        public static void RunRuntimeReflectionConstInt2()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_int2");
        }

        public static void RunRuntimeReflectionConstLong()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_long");
        }

        public static void RunRuntimeReflectionConstLong2()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_long2");
        }

        public static void RunRuntimeReflectionConstFloat()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_float");
        }

        public static void RunRuntimeReflectionConstDouble()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "const_double");
        }

        public static void RunRuntimeReflectionProperty()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "property");
        }

        public static void RunRuntimeReflectionGetExecutingAssembly()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "GetExecutingAssembly");
        }

        public static void RunRuntimeReflectionGetCurrentMethod()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "GetMethodBaseCurrentMethod");
        }

        public static void RunRuntimeReflectionGetNotCtorCCtorMethods()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "GetNotCtorCCtorMethods");
        }

        public static void RunRuntimeReflectionGetCCtorMethod()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_Reflection), "GetCCtorMethod");
        }

        public static void RunRuntimeDelegateReflectionProbe()
        {
            System.Span<byte> utf8Name = stackalloc byte[3];
            int written = System.Text.Encoding.UTF8.GetBytes("Run", utf8Name);
            Assert.Equal(3, written);
            Assert.Equal(82, (int)utf8Name[0]);
            Assert.Equal(117, (int)utf8Name[1]);
            Assert.Equal(110, (int)utf8Name[2]);
            System.Span<byte> expectedUtf8Name = stackalloc byte[3];
            expectedUtf8Name[0] = 82;
            expectedUtf8Name[1] = 117;
            expectedUtf8Name[2] = 110;
            Assert.IsTrue(System.MemoryExtensions.SequenceEqual<byte>(utf8Name, expectedUtf8Name));

            System.Type type = typeof(Tests.Fixtures.FT_AOT_Class);
            System.Reflection.BindingFlags flags =
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static;

            System.Reflection.MethodInfo[] methods = type.GetMethods(flags);
            bool found = false;
            for (int i = 0; i < methods.Length; i++)
            {
                if (methods[i].Name == "Run")
                {
                    found = true;
                    break;
                }
            }
            Assert.IsTrue(found);

            System.Reflection.MethodInfo method = type.GetMethod("Run", flags);
            if (method == null)
            {
                Assert.Fail("FT_AOT_Class.GetMethod(Run) returned null; GetMethods count=" + methods.Length);
            }
            Assert.Equal("Run", method.Name);
        }

        public static void RunRuntimeString()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_String));
        }

        public static void RunRuntimeStringEmptyString()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "EmptyString");
        }

        public static void RunRuntimeStringCreateStringFromCharArray()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromCharArray");
        }

        public static void RunRuntimeStringCreateStringFromCharArrayWithOffset()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromCharArrayWithOffset");
        }

        public static void RunRuntimeStringCreateStringFromCharPtr()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromCharPtr");
        }

        public static void RunRuntimeStringCreateStringFromCharPtrWithOffset()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromCharPtrWithOffset");
        }

        public static void RunRuntimeStringCreateStringFromSBytePtr()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromSBytePtr");
        }

        public static void RunRuntimeStringCreateStringFromSBytePtrWithOffset()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromSBytePtrWithOffset");
        }

        public static void RunRuntimeStringCreateStringFromSBytePtrWithOffsetAndEncoding()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_String), "CreateStringFromSBytePtrWithOffsetAndEncoding");
        }

        public static void RunRuntimeEnum()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Enum));
        }

        public static void RunRuntimeInterlocked()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Interlocked));
        }

        public static void RunRuntimeUsing()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_using));
        }

        public static void RunRuntimeForeach()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_foreach));
        }

        public static void RunRuntimeEvent()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_event));
        }

        public static void RunRuntimeNullable()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Nullable));
        }

        public static void RunRuntimeInterfaceDefaultMethod()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_InterfaceDefaultMethod));
        }

        public static void RunRuntimeMarshal()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_Marshal));
        }

        public static void RunRuntimeAotTypeImplInterpInterface()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_AOTTypeImplInterpInterface));
        }

        public static void RunRuntimeDynamicPlaceholder()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.Dynamics.TC_dynamic));
        }

        public static void RunRuntimeArrayGenericInterface()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.TC_ArrayGenericInterface));
        }

        public static void RunRuntimeArrayGenericInterfaceClassGet()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_ArrayGenericInterface), "class_get_1");
        }

        public static void RunRuntimeArrayGenericInterfaceClassSet()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_ArrayGenericInterface), "class_set_1");
        }

        public static void RunRuntimeArrayGenericInterfaceStructGet()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_ArrayGenericInterface), "struct_get_1");
        }

        public static void RunRuntimeArrayGenericInterfaceStructSet()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.CSharp.TC_ArrayGenericInterface), "struct_set_1");
        }

        public static void RunRuntimeDelegateOpenClose()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.Delegates.TC_Delegate_OpenClose));
        }

        public static void RunRuntimeDelegateDynamicInvoke()
        {
            LegacyTestRunner.RunType(typeof(Tests.CSharp.Delegates.TC_Delegate_DynamicInvoke));
        }
    }
}
