using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Tests.Instruments.Funcs
{
    internal class TC_callvir_aot : GeneralTestCaseBase
    {
        static string GetType2<T>(T x)
        {
            return x.GetType().Name;
        }

        public static int Calc<T>(T x) where T : IFoo
        {
            return x.Calc();
        }
        
        public static int GetValueHashCode<T>(T x)
        {
            return x.GetHashCode();
        }

        [UnitTest]
        public void class_call_virtual()
        {
            var x = new ForFunClass(1, 2);
            Assert.Equal(3, x.Calc());
        }

        [UnitTest]
        public void class_call_interface()
        {
            IFoo x = new ForFunClass(1, 2);
            Assert.Equal(3, x.Calc());
        }

        [UnitTest]
        public void class_cons_all_impl()
        {
            var x = new ForFunClass(1, 2);
            Assert.Equal(3, Calc(x));
        }

        [UnitTest]
        public void class_cons_all_not_impl()
        {
            var x = new ForFunClass(1, 2);
            Assert.Equal("ForFunClass", GetType2(x));
        }

        /// TODO BUG OF il2cpp
        [UnitTest]
        public void class_generic_interface_covariant()
        {
            IBar<object> x = new ForBarClass2();
            // mono and il2cpp return 1
#if UNITY_EDITOR
            Assert.Equal(1, x.Sum(0));
#else
            // leanclr and coreclr return 2
            Assert.Equal("2", x.Sum(0));
#endif
        }

        /// TODO BUG OF il2cpp
        [UnitTest]
        public void class_generic_interface_contravariant()
        {
            IRun<string> x = new ForBarClass2();
#if UNITY_EDITOR
            Assert.Equal(1, x.Comput(""));
#else
            Assert.Equal(2, x.Comput(""));
#endif
        }

        [UnitTest]
        public void struct_cons_all_impl()
        {
            var x = new ForFunValue(1, 2);
            Assert.Equal(3, Calc(x));
        }

        [UnitTest]
        public void struct_cons_all_not_impl()
        {
            var x = new ForFunValue(1, 2);
            Assert.Equal("ForFunValue", GetType2(x));
        }

        [UnitTest]
        public void struct_contraint_hashcode()
        {
            var a = new ForInherenGenericAotValue() { x = 1 };
            Assert.Equal(1, GetValueHashCode(a));
        }

    }
}
