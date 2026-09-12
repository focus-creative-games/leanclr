namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibRuntimeTypeHandle()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_RuntimeTypeHandle));
            LegacyTestRunner.RunType(typeof(CorlibRuntimeTypeHandleNet10Semantics));
        }

        public static void RunCorlibRuntimeTypeHandleNet10Semantics()
        {
            LegacyTestRunner.RunType(typeof(CorlibRuntimeTypeHandleNet10Semantics));
        }
    }

    internal sealed class CorlibRuntimeTypeHandleNet10Semantics
    {
        [UnitTest]
        public void RuntimeTypeIsResolvableFromCoreLibAssembly()
        {
            System.Type runtimeType = typeof(System.RuntimeTypeHandle).Assembly.GetType("System.RuntimeType");
            Assert.NotNull(runtimeType);
        }

        [UnitTest]
        public void HasInstantiationPrivateMethodWasRemovedInNet10()
        {
            System.Type runtimeType = typeof(System.RuntimeTypeHandle).Assembly.GetType("System.RuntimeType");
            Assert.NotNull(runtimeType);

            System.Reflection.MethodInfo method = typeof(System.RuntimeTypeHandle).GetMethod(
                "HasInstantiation",
                System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static,
                null,
                new[] { runtimeType },
                null);
            Assert.Equal(null, method);
        }

        [UnitTest]
        public void IsGenericTypeMatchesNet10ReplacementForHasInstantiation()
        {
            Assert.Equal(false, typeof(int).IsGenericType);
            Assert.Equal(false, typeof(bool).IsGenericType);
            Assert.Equal(false, typeof(string).IsGenericType);

            Assert.Equal(true, typeof(System.Collections.Generic.List<>).IsGenericType);
            Assert.Equal(true, typeof(System.Collections.Generic.Dictionary<,>).IsGenericType);
            Assert.Equal(true, typeof(System.Collections.Generic.List<int>).IsGenericType);
            Assert.Equal(true, typeof(System.Collections.Generic.Dictionary<string, int>).IsGenericType);
            Assert.Equal(true, typeof(int?).IsGenericType);
            Assert.Equal(true, typeof(System.Action<int>).IsGenericType);
            Assert.Equal(true, typeof(System.Func<int, string>).IsGenericType);
            Assert.Equal(true, typeof(RuntimeTypeHandleGenericHelper<>.NestedGeneric<>).IsGenericType);
            Assert.Equal(true, typeof(RuntimeTypeHandleGenericHelper<int>.NestedGeneric<string>).IsGenericType);
        }

        [UnitTest]
        public void IsGenericTypeIsFalseForGenericParametersArraysPointersAndByRefs()
        {
            System.Type genericParameter = typeof(System.Collections.Generic.List<>).GetGenericArguments()[0];
            Assert.IsTrue(genericParameter.IsGenericParameter);
            Assert.Equal(false, genericParameter.IsGenericType);

            Assert.Equal(false, typeof(int[]).IsGenericType);
            Assert.Equal(false, typeof(int[,]).IsGenericType);
            Assert.Equal(false, typeof(string[]).IsGenericType);
            Assert.Equal(false, typeof(System.Collections.Generic.List<int>[]).IsGenericType);
            Assert.Equal(false, typeof(System.Collections.Generic.List<int>[,]).IsGenericType);
            Assert.Equal(false, typeof(int*).IsGenericType);

            Assert.Equal(false, typeof(int).MakeByRefType().IsGenericType);
            Assert.Equal(false, typeof(string).MakeByRefType().IsGenericType);
            Assert.Equal(false, typeof(System.Collections.Generic.List<int>).MakeByRefType().IsGenericType);
            Assert.Equal(false, typeof(System.Collections.Generic.List<>).MakeByRefType().IsGenericType);
            Assert.Equal(false, genericParameter.MakeByRefType().IsGenericType);
        }

        private sealed class RuntimeTypeHandleGenericHelper<T>
        {
            public sealed class NestedGeneric<U>
            {
            }
        }
    }
}
