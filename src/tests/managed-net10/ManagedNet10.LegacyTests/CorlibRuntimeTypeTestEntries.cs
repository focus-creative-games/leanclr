namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibRuntimeType()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_RuntimeType));
            LegacyTestRunner.RunType(typeof(CorlibRuntimeTypeNet10Semantics));
        }

        public static void RunCorlibRuntimeTypeAfterReflectionCaches()
        {
            RunCorlibReflectionRuntimeMethodInfo();
            RunCorlibReflectionRuntimeParameterInfo();
            RunCorlibReflectionRuntimePropertyInfo();
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Reflection_RuntimeModule));
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Runtime_CompilerServices_RuntimeHelper));
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_GCHandle));
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Runtime_InteropServices_RuntimeInformation));
            RunCorlibRuntimeType();
        }

        public static void RunCorlibRuntimeTypeLegacy()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_RuntimeType));
        }

        public static void RunCorlibRuntimeTypeNet10Semantics()
        {
            LegacyTestRunner.RunType(typeof(CorlibRuntimeTypeNet10Semantics));
        }

        public static void RunCorlibRuntimeTypeGetNestedTypeByName()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_RuntimeType),
                "GetNestedType_ByName_ReturnsMatchingType");
        }

        public static void RunCorlibRuntimeTypeGetNestedTypeIgnoreCase()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_RuntimeType),
                "GetNestedType_IgnoreCase_FindsPublicNestedType");
        }
    }

    internal sealed class CorlibRuntimeTypeNet10Semantics
    {
        [UnitTest]
        public void GetNestedTypesPublicReturnsPublicNestedTypesOnly()
        {
            System.Type[] nested = typeof(CorlibTests.InternalCall.TC_System_RuntimeType).GetNestedTypes(System.Reflection.BindingFlags.Public);
            if (nested == null)
            {
                throw new System.Exception("GetNestedTypes(Public) returned null.");
            }
            if (nested.Length != 2)
            {
                throw new System.Exception("GetNestedTypes(Public) returned unexpected length: " + nested.Length);
            }
            RequireContainsTypeName(nested, "PublicNestedType");
            RequireContainsTypeName(nested, "PublicNestedStruct");
            RequireDoesNotContainTypeName(nested, "PrivateNestedType");
        }

        [UnitTest]
        public void GetNestedTypesPublicAndNonPublicReturnsAllNestedTypes()
        {
            System.Type[] nested = typeof(CorlibTests.InternalCall.TC_System_RuntimeType).GetNestedTypes(
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic);
            if (nested == null)
            {
                throw new System.Exception("GetNestedTypes(Public | NonPublic) returned null.");
            }
            if (nested.Length < 4)
            {
                throw new System.Exception("GetNestedTypes(Public | NonPublic) returned unexpected length: " + nested.Length);
            }
            RequireContainsTypeName(nested, "PublicNestedType");
            RequireContainsTypeName(nested, "PrivateNestedType");
            RequireContainsTypeName(nested, "PublicNestedStruct");
            RequireContainsTypeName(nested, "PrivateNestedStaticClass");
        }

        private static void RequireContainsTypeName(System.Type[] types, string name)
        {
            if (!ContainsTypeName(types, name))
            {
                throw new System.Exception("Nested type array did not contain " + name);
            }
        }

        private static void RequireDoesNotContainTypeName(System.Type[] types, string name)
        {
            if (ContainsTypeName(types, name))
            {
                throw new System.Exception("Nested type array unexpectedly contained " + name);
            }
        }

        private static bool ContainsTypeName(System.Type[] types, string name)
        {
            for (int i = 0; i < types.Length; i++)
            {
                if (types[i] == null)
                {
                    throw new System.Exception("Nested type array contains null at index " + i);
                }
                if (types[i].Name == name)
                {
                    return true;
                }
            }

            return false;
        }
    }
}
