using System;
using System.Collections.Generic;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    internal class TC_System_RuntimeTypeHandle : TestCaseBase
    {
#if IL2CPP_ONLY
        [UnitTest]
        public void ByRefLike()
        {
            var t = typeof(int);
            Assert.False(t.IsByRefLike);
        }

        [UnitTest]
        public void ByRefLike_Span()
        {
            var t = typeof(Span<int>);
            Assert.True(t.IsByRefLike);
        }
#endif

        // --- System.RuntimeTypeHandle::HasInstantiation ---

        [UnitTest]
        public void HasInstantiation_Primitive_False()
        {
            AssertHasInstantiation(false, typeof(int));
            AssertHasInstantiation(false, typeof(bool));
            AssertHasInstantiation(false, typeof(string));
        }

        [UnitTest]
        public void HasInstantiation_OpenGenericDefinition_True()
        {
            AssertHasInstantiation(true, typeof(List<>));
            AssertHasInstantiation(true, typeof(Dictionary<,>));
            AssertHasInstantiation(true, typeof(HasInstantiationHelperGeneric<>));
        }

        [UnitTest]
        public void HasInstantiation_ClosedGeneric_True()
        {
            AssertHasInstantiation(true, typeof(List<int>));
            AssertHasInstantiation(true, typeof(Dictionary<string, int>));
            AssertHasInstantiation(true, typeof(HasInstantiationHelperGeneric<string>));
        }

        [UnitTest]
        public void HasInstantiation_NestedAndDelegate_True()
        {
            AssertHasInstantiation(true, typeof(Action<int>));
            AssertHasInstantiation(true, typeof(Func<int, string>));
            AssertHasInstantiation(true, typeof(HasInstantiationHelperGeneric<>.NestedGeneric<>));
            AssertHasInstantiation(true, typeof(HasInstantiationHelperGeneric<int>.NestedGeneric<string>));
        }

        [UnitTest]
        public void HasInstantiation_Nullable_True()
        {
            AssertHasInstantiation(true, typeof(int?));
            AssertHasInstantiation(true, typeof(HasInstantiationHelperEnum?));
        }

        [UnitTest]
        public void HasInstantiation_GenericTypeParameter_True()
        {
            Type openList = typeof(List<>);
            Type typeParam = openList.GetGenericArguments()[0];
            AssertHasInstantiation(false, typeParam);
            Assert.IsTrue(typeParam.IsGenericParameter);
        }

        [UnitTest]
        public void HasInstantiation_Arrays_False()
        {
            AssertHasInstantiation(false, typeof(int[]));
            AssertHasInstantiation(false, typeof(int[,]));
            AssertHasInstantiation(false, typeof(string[]));
            AssertHasInstantiation(false, typeof(List<int>[]));
            AssertHasInstantiation(false, typeof(List<int>[,]));
        }

        [UnitTest]
        public void HasInstantiation_Pointers_False()
        {
            AssertHasInstantiation(false, typeof(int*));
        }

        [UnitTest]
        public void HasInstantiation_IntByRef_False()
        {
            Type intByRef = typeof(int).MakeByRefType();
            Assert.IsTrue(intByRef.IsByRef);
            AssertHasInstantiation(false, intByRef);
            Assert.Equal(false, intByRef.IsGenericType);
        }

        [UnitTest]
        public void HasInstantiation_StringByRef_False()
        {
            Type stringByRef = typeof(string).MakeByRefType();
            Assert.IsTrue(stringByRef.IsByRef);
            AssertHasInstantiation(false, stringByRef);
        }

        [UnitTest]
        public void HasInstantiation_ListIntByRef_False()
        {
            Type listIntByRef = typeof(List<int>).MakeByRefType();
            Assert.IsTrue(listIntByRef.IsByRef);
            AssertHasInstantiation(false, listIntByRef);
            Assert.Equal(false, listIntByRef.IsGenericType);
        }

        [UnitTest]
        public void HasInstantiation_OpenGenericByRef_False()
        {
            Type openGenericByRef = typeof(List<>).MakeByRefType();
            Assert.IsTrue(openGenericByRef.IsByRef);
            AssertHasInstantiation(false, openGenericByRef);
        }

        [UnitTest]
        public void HasInstantiation_ClosedGenericNestedByRef_False()
        {
            Type closedByRef = typeof(Dictionary<string, int>).MakeByRefType();
            Assert.IsTrue(closedByRef.IsByRef);
            AssertHasInstantiation(false, closedByRef);
        }

        [UnitTest]
        public void HasInstantiation_GenericTypeParameterByRef_False()
        {
            Type typeParam = typeof(List<>).GetGenericArguments()[0];
            Type typeParamByRef = typeParam.MakeByRefType();
            Assert.IsTrue(typeParamByRef.IsByRef);
            AssertHasInstantiation(false, typeParamByRef);
        }

        [UnitTest]
        public void HasInstantiation_MatchesIsGenericType_ForNonByRef()
        {
            Type[] types =
            {
                typeof(int),
                typeof(List<>),
                typeof(List<int>),
                typeof(int?),
                typeof(int[]),
                typeof(List<int>[]),
            };
            foreach (Type t in types)
            {
                Assert.Equal(t.IsGenericType, InvokeHasInstantiation(t));
            }
        }

        private static void AssertHasInstantiation(bool expected, Type type)
        {
            bool actual = InvokeHasInstantiation(type);
            if (actual != expected)
            {
                Assert.Fail(string.Format("HasInstantiation({0}): expected {1}, got {2}", type.FullName, expected, actual));
            }
        }

        private static bool InvokeHasInstantiation(Type type)
        {
            Type runtimeType = typeof(RuntimeTypeHandle).Assembly.GetType("System.RuntimeType");
            Assert.NotNull(runtimeType);
            MethodInfo method = typeof(RuntimeTypeHandle).GetMethod(
                "HasInstantiation",
                BindingFlags.NonPublic | BindingFlags.Static,
                null,
                new[] { runtimeType },
                null);
            Assert.NotNull(method);
            return (bool)method.Invoke(null, new object[] { type });
        }

        private enum HasInstantiationHelperEnum
        {
            A,
            B,
        }

        private class HasInstantiationHelperGeneric<T>
        {
            public class NestedGeneric<U>
            {
            }
        }
    }
}
