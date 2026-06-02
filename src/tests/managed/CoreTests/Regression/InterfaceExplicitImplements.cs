
using test;



namespace Tests.Bugs
{
    public interface IInterpBase1
    {
        int Value { get; }
    }

    public interface IInterpBase2 : IInterpBase1
    {
        int Value2 { get; }

#if UNITY_2021_1_OR_NEWER
        int IInterpBase1.Value => Value2;
#endif
    }

    public interface IInterpGenericBase1<T>
    {
        T Value { get; }
    }

    public interface IInterpGenericBase2<T> : IInterpGenericBase1<T>
    {
        T Value2 { get; }

#if UNITY_2021_1_OR_NEWER
        T IInterpGenericBase1<T>.Value => Value2;
#endif
    }

//    public class InterpImplAOT1 : IAOTBase2
//    {
//        public int x = 0;

//        public int Value2 => x;

//#if !UNITY_2021_1_OR_NEWER
//        int IAOTBase1.Value => Value2;
//#endif
//    }

//    public struct InterpImplAOT2 : IAOTBase2
//    {
//        public int x;

//        public int Value2 => x;

//#if !UNITY_2021_1_OR_NEWER
//        int IAOTBase1.Value => Value2;
//#endif
//    }

//    public class InterpImplAOTGeneric1 : IAOTGenericBase2<int>
//    {
//        public int x = 0;

//        public int Value2 => x;

//#if !UNITY_2021_1_OR_NEWER
//        int IAOTGenericBase1<int>.Value => Value2;
//#endif
//    }

//    public struct InterpImplAOTGeneric2 : IAOTGenericBase2<int>
//    {
//        public int x;

//        public int Value2 => x;

//#if !UNITY_2021_1_OR_NEWER
//        int IAOTGenericBase1<int>.Value => Value2;
//#endif
//    }

    public class InterpImplInterp1 : IInterpBase2
    {
        public int x = 0;

        public int Value2 => x;

#if !UNITY_2021_1_OR_NEWER
        int IInterpBase1.Value => Value2;
#endif
    }

    public struct InterpImplInterp2 : IInterpBase2
    {
        public int x;

        public int Value2 => x;

#if !UNITY_2021_1_OR_NEWER
        int IInterpBase1.Value => Value2;
#endif
    }

    public class InterpImplInterpGeneric1 : IInterpGenericBase2<int>
    {
        public int x = 0;

        public int Value2 => x;

#if !UNITY_2021_1_OR_NEWER
        int IInterpGenericBase1<int>.Value => Value2;
#endif
    }

    public struct InterpImplInterpGeneric2 : IInterpGenericBase2<int>
    {
        public int x;

        public int Value2 => x;

#if !UNITY_2021_1_OR_NEWER
        int IInterpGenericBase1<int>.Value => Value2;
#endif
    }

    public class InterfaceExplicitImplements : GeneralTestCaseBase
    {

        //[UnitTest]
        //public void ClassImplAOT()
        //{
        //    IAOTBase1 c = new InterpImplAOT1() { x = 1 };
        //    Assert.Equals(c.Value, 1);
        //}

        //[UnitTest]
        //public void StructImplAOT()
        //{
        //    IAOTBase1 c = new InterpImplAOT2 { x = 1 };
        //    Assert.Equals(c.Value, 1);
        //}

        //[UnitTest]
        //public void ClassImplAOTGeneric()
        //{
        //    IAOTGenericBase1<int> c = new InterpImplAOTGeneric1() { x = 1 };
        //    Assert.Equals(c.Value, 1);
        //}

        //[UnitTest]
        //public void StructImplAOTGeneric()
        //{
        //    IAOTGenericBase1<int> c = new InterpImplAOTGeneric2 { x = 1 };
        //    Assert.Equals(c.Value, 1);
        //}

        [UnitTest]
        public void ClassImplInterp()
        {
            IInterpBase1 c = new InterpImplInterp1() { x = 1 };
            Assert.Equals(c.Value, 1);
        }

        [UnitTest]
        public void StructImplInterp()
        {
            IInterpBase1 c = new InterpImplInterp2 { x = 1 };
            Assert.Equals(c.Value, 1);
        }

        [UnitTest]
        public void ClassImplInterpGeneric()
        {
            IInterpGenericBase1<int> c = new InterpImplInterpGeneric1() { x = 1 };
            Assert.Equals(c.Value, 1);
        }

        [UnitTest]
        public void StructImplInterpGeneric()
        {
            IInterpGenericBase1<int> c = new InterpImplInterpGeneric2 { x = 1 };
            Assert.Equals(c.Value, 1);
        }

#if UNITY_2021_3_OR_NEWER

        interface ITestBase
        {
            int Value { get; }
        }

        interface ITestDerived1 : ITestBase
        {
            int ITestBase.Value => 1;

            //int Value2 => 10;
        }

        interface ITestDerived2 : ITestBase
        {
            int ITestBase.Value => 2;

            //int Value2 => 20;
        }

        class TestImpl : ITestDerived1, ITestDerived2
        {
            int ITestBase.Value => 3;

            //public int Value2 => 30;

            public int GetBaseValue()
            {
                int a = ((ITestBase)this).Value;
                return a;
            }
        }

        [UnitTest]
        public void MultipleExplicitImpls1()
        {
            ITestBase c = new TestImpl();
            Assert.Equals(c.Value, 3);
        }

        [UnitTest]
        public void MultipleExplicitImpls2()
        {
            var c = new TestImpl();
            Assert.Equals(3, c.GetBaseValue());
        }

        [UnitTest]
        public void AOTMultipleExplicitImpls1()
        {
            AOTDefs.ITestBase c = new AOTDefs.TestImpl();
            Assert.Equals(c.Value, 3);
        }

        [UnitTest]
        public void AOTMultipleExplicitImpls2()
        {
            var c = new AOTDefs.TestImpl();
            Assert.Equals(3, c.GetBaseValue());
        }
#endif

        //[UnitTest]
        //public void OverrideMultipleInterfaceImpls1()
        //{
        //    ITestDerived1 c = new TestImpl();
        //    Assert.Equals(c.Value2, 10);
        //}

        //[UnitTest]
        //public void OverrideMultipleInterfaceImpls2()
        //{
        //    ITestDerived2 c = new TestImpl();
        //    Assert.Equals(c.Value2, 20);
        //}

        //[UnitTest]
        //public void OverrideMultipleInterfaceImpls3()
        //{
        //    var c = new TestImpl();
        //    Assert.Equals(c.Value2, 30);
        //}
    }
}
