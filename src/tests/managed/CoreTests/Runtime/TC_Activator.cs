using test;
using System;
using System.Collections.Generic;

namespace Tests.CSharp
{
    class TC_Activator : GeneralTestCaseBase
    {

        private T Create<T>() where T : new()
        {
            return new T();
        }

        class ClassA
        {
            public int x;

            public static int GetX(ClassA a)
            {
                return a.x;
            }
        }

        struct StructA
        {
            public int x;

            public static int GetX(StructA a)
            {
                return a.x;
            }
        }

        int GetFirstArgX(int a, ClassA b)
        {
            return a; 
        }

        int GetFirstArgX(int a, StructA b)
        {
            return a;
        }

        [UnitTest]
        public void ActivatorCreateInstanceClass()
        {
            var instance = Create<ClassA>();
            Assert.NotNull(instance);
            Assert.Equal("ClassA", instance.GetType().Name);
        }

        [UnitTest]
        public void ActivatorCreateInstanceStruct()
        {
            var instance = Create<StructA>();
            Assert.Equal(0, instance.x);
        }

        [UnitTest]
        public void AOTActivatorCreateInstanceClass()
        {
            var instance = AOTDefs.AOTActivator.Create<AOTDefs.AOTActivator.ClassA>();
            Assert.NotNull(instance);
            Assert.Equal("ClassA", instance.GetType().Name);
        }

        [UnitTest]
        public void AOTActivatorCreateInstanceStruct()
        {
            var instance = AOTDefs.AOTActivator.Create<AOTDefs.AOTActivator.StructA>();
            Assert.Equal(0, instance.x);
        }

        [UnitTest]
        public void TestStackAfterActivatorCreateInstanceClass()
        {
            int x = GetFirstArgX(42, System.Activator.CreateInstance<ClassA>());
            Assert.Equal(42, x);
        }

        [UnitTest]
        public void TestStackAfterActivatorCreateInstanceStruct()
        {
            int x = GetFirstArgX(42, System.Activator.CreateInstance<StructA>());
            Assert.Equal(42, x);
        }

        [UnitTest]
        public void NewObjZeroArgument()
        {
            var instance = new Dictionary<int, StructA>();
            Assert.NotNull(instance);
        }

        [UnitTest]
        public void CreateInt()
        {
            "abc".ToString();
            int x = System.Activator.CreateInstance<byte>();
            Assert.Equal(0, x);
        }
    }
}
