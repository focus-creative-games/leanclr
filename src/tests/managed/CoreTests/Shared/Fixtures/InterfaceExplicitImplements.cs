using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public interface IAOTBase1
    {
        int Value { get; }
    }

    public interface IAOTBase2 : IAOTBase1
    {
        int Value2 { get; }

#if UNITY_2021_1_OR_NEWER
        int IAOTBase1.Value => Value2;
#endif
    }

    public interface IAOTGenericBase1<T>
    {
        T Value { get; }
    }

    public interface IAOTGenericBase2<T> : IAOTGenericBase1<T>
    {
        T Value2 { get; }
#if UNITY_2021_1_OR_NEWER
        T IAOTGenericBase1<T>.Value => Value2;
#endif
    }

    //public class AOTImpl3 : IAOTBase2
    //{
    //    public int x = 0;

    //    public int Value2 => x;
    //}

    //public struct AOTImpl4 : IAOTBase2
    //{
    //    public int x;

    //    public int Value2 => x;
    //}

#if UNITY_2021_3_OR_NEWER

    public interface ITestBase
    {
        int Value { get; }
    }

    public interface ITestDerived1 : ITestBase
    {
        int ITestBase.Value => 1;

        //int Value2 => 10;
    }

    public interface ITestDerived2 : ITestBase
    {
        int ITestBase.Value => 2;

        //int Value2 => 20;
    }

    public class TestImpl : ITestDerived1, ITestDerived2
    {
        int ITestBase.Value => 3;

        //public int Value2 => 30;

        public int GetBaseValue()
        {
            int a = ((ITestBase)this).Value;
            return a;
        }
    }
#endif
}
