using Tests.Fixtures;

namespace Tests.Instruments.Funcs
{
    internal class TC_callvir_interp : TestCaseBase
    {
        private static string GetType2<T>(T x)
        {
            return x.GetType().Name;
        }

        public static int Calc<T>(T x) where T : IInterpFoo
        {
            return x.Calc();
        }

        public static int GetValueHashCode<T>(T x)
        {
            return x.GetHashCode();
        }

        [UnitTest]
        public void class_virtual()
        {
            var x = new ForInhereInterpClass(10, 20);
            Assert.Equal(11, x.Calc());
        }

        [UnitTest]
        public void class_interface()
        {
            IInterpFoo x = new ForInhereInterpClass(10, 20);
            Assert.Equal(11, x.Calc());
        }

        [UnitTest]
        public void class_cons_all_impl()
        {
            var x = new ForInhereInterpValue(10, 20);
            Assert.Equal(11, Calc(x));
        }

        [UnitTest]
        public void class_generic_interface_covariant()
        {
            IInterpBar<object> x = new ForInherenGenericInterpValue { x = 2 };
            Assert.Equal(2, x.Sum(0));
        }

        [UnitTest]
        public void class_generic_interface_contravariant()
        {
            IInterpRun<string> x = new ForInherenGenericInterpValue { x = 2 };
            Assert.Equal(2, x.Comput(""));
        }

        [UnitTest]
        public void struct_call_instance()
        {
            var x = new ForInhereInterpValue(10, 20);
            Assert.Equal(10, x.Show());
        }

        [UnitTest]
        public void struct_call_interface()
        {
            IInterpFoo x = new ForInhereInterpValue(10, 20);
            Assert.Equal(11, x.Calc());
        }

        [UnitTest]
        public void struct_cons_all_impl()
        {
            var x = new ForInhereInterpValue(10, 20);
            Assert.Equal(11, Calc(x));
        }

        [UnitTest]
        public void struct_cons_all_not_impl()
        {
            var x = new ForInhereInterpValue(1, 2);
            Assert.Equal("ForInhereInterpValue", GetType2(x));
        }

        [UnitTest]
        public void struct_contraint_hashcode()
        {
            var a = new ForInherenGenericAotValue { x = 1 };
            Assert.Equal(1, GetValueHashCode(a));
        }

        [UnitTest]
        public void struct_contraint_tostring()
        {
            var a = new ForInherenGenericAotValue { x = 1 };
            Assert.Equal(a.ToString(), "1");
        }

        private static Vector2 color;

        [UnitTest]
        public void DuplicateBigStruct_Return1()
        {
            Vector2 c1 = default;
            color = color = c1;
        }
    }
}
