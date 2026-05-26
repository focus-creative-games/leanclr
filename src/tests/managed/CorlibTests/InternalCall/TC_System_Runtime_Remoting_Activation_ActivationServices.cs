using System;
using System.Collections.Generic;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    #if IL2CPP_ONLY
    internal class TC_System_Runtime_Remoting_Activation_ActivationServices : GeneralTestCaseBase
    {
        private class UninitializedClassInstanceTarget
        {
            public int marker;

            public UninitializedClassInstanceTarget()
            {
                marker = 42;
            }
        }

        private struct UninitializedStructTarget
        {
            public int value;
            public bool flag;

            public UninitializedStructTarget(int v)
            {
                value = v;
                flag = true;
            }
        }

        private abstract class AbstractTarget
        {
        }

        private interface ITarget
        {
        }

        private static MethodInfo GetAllocateMethod()
        {
            Type activationServicesType = typeof(object).Assembly.GetType("System.Runtime.Remoting.Activation.ActivationServices");
            Assert.NotNull(activationServicesType);
            MethodInfo method = activationServicesType.GetMethod(
                "AllocateUninitializedClassInstance",
                BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic,
                null,
                new[] { typeof(Type) },
                null);
            Assert.NotNull(method);
            return method;
        }

        private static object Allocate(Type type)
        {
            return GetAllocateMethod().Invoke(null, new object[] { type });
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_NullType_ReturnsNull()
        {
            Assert.Null(Allocate(null));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_ReferenceType_DefaultFieldsCtorNotRun()
        {
            object obj = Allocate(typeof(UninitializedClassInstanceTarget));
            Assert.NotNull(obj);
            Assert.Equal(typeof(UninitializedClassInstanceTarget), obj.GetType());

            var instance = (UninitializedClassInstanceTarget)obj;
            Assert.Equal(0, instance.marker);
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_ValueType_ReturnsBoxedDefault()
        {
            object obj = Allocate(typeof(UninitializedStructTarget));
            Assert.NotNull(obj);
            Assert.Equal(typeof(UninitializedStructTarget), obj.GetType());

            var value = (UninitializedStructTarget)obj;
            Assert.Equal(0, value.value);
            Assert.IsFalse(value.flag);
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_String_ReturnsEmptyString()
        {
            Assert.ExpectException<ArgumentException>(() => Allocate(typeof(string)));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_SzArray_ReturnsZeroLengthArray()
        {
            Assert.ExpectException<ArgumentException>(() => Allocate(typeof(int[])));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_PrimitiveInt_ReturnsBoxedZero()
        {
            object obj = Allocate(typeof(int));
            Assert.NotNull(obj);
            Assert.Equal(typeof(int), obj.GetType());
            Assert.Equal(0, obj);
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_ObjectType_ReturnsNullReference()
        {
            object obj = Allocate(typeof(object));
            Assert.NotNull(obj);
            Assert.Equal(typeof(object), obj.GetType());
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_AbstractType_ThrowsArgumentException()
        {
            Assert.ExpectException<ArgumentException>(() => Allocate(typeof(AbstractTarget)));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_InterfaceType_ThrowsArgumentException()
        {
            Assert.ExpectException<ArgumentException>(() => Allocate(typeof(ITarget)));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_OpenGenericDefinition_ThrowsArgumentException()
        {
            Assert.ExpectException<ArgumentException>(() => Allocate(typeof(List<>).GetGenericTypeDefinition()));
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_ClosedGeneric_Works()
        {
            object obj = Allocate(typeof(List<int>));
            Assert.NotNull(obj);
            Assert.Equal(typeof(List<int>), obj.GetType());
        }

        [UnitTest]
        public void AllocateUninitializedClassInstance_ByRefType_ThrowsArgumentException()
        {
            Type byRefType = typeof(int).MakeByRefType();
            Assert.ExpectException<ArgumentException>(() => Allocate(byRefType));
        }
    }
#endif
}
