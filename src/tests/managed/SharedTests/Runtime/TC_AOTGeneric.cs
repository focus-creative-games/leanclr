
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp.AOTGenerics
//{
//    struct A
//    {
//        public int x;
//        public float y;
//        public string z;
//    }

//    public struct SkillEquipInfo
//    {
//        public int pos_1_skill;
//        public int pos_2_skill;
//        public int pos_3_skill;
//    }

//    public struct IntVec : IEquatable<IntVec>
//    {
//        public int x;
//        public int y;
//        public int z;

//        public IntVec(int a, int b, int c)
//        {
//            x = a;
//            y = b;
//            z = c;
//        }

//        public override bool Equals(object other)
//        {
//            return Equals((IntVec)other);
//        }

//        public bool Equals(IntVec other)
//        {
//            return x == other.x && y == other.y && z == other.z;
//        }

//        public override int GetHashCode()
//        {
//            return x + y + z;
//        }

//        public override string ToString()
//        {
//            return $"({x},{y},{z})";
//        }
//    }

//    internal class TC_AOTGeneric : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void test_1()
//        {
//            var arr = new List<A>();
//            arr.Add(new A { x = 1, y = 2f, z = "abc" });
//            var e = arr[0];
//            Assert.Equal(1, e.x);
//            Assert.Equal(2f, e.y);
//            Assert.Equal("abc", e.z);
//        }

//        [UnitTest]
//        public void test_2()
//        {
//            var a = new List<A>() { new A { x = 1, y = 2f, z = "abc" } };
//            var b = new List<A>() { new A { x = 2, y = 4f, z = "aaa" } };
//            a.AddRange(b);
//            Assert.Equal(2, a.Count);
//            var e = a[1];
//            Assert.Equal(2, e.x);
//            Assert.Equal(4f, e.y);
//            Assert.Equal("aaa", e.z);
//        }

//        [UnitTest]
//        public void test_3()
//        {
//            var a = new List<A>() { new A { x = 1, y = 2f, z = "abc" } };
//            var b = new A[] { new A { x = 2, y = 4f, z = "aaa" } };
//            a.AddRange(b);
//            Assert.Equal(2, a.Count);
//            var e = a[1];
//            Assert.Equal(2, e.x);
//            Assert.Equal(4f, e.y);
//            Assert.Equal("aaa", e.z);
//        }

//        [UnitTest]
//        public void test_call_aot_method_as_interp()
//        {
//            var a = new ValueTuple<int, float, IntVec, double, ushort, Vector3d>(
//                1,
//                2.0f,
//                new IntVec(1,2,3),
//                3.0,
//                15,
//                new Vector3d()
//                );
//            var b = a;
//            Assert.Equal(a, b);
//        }

//        [UnitTest]
//        public void test_call_aot_vir_method_as_interp()
//        {
//            IDictionary<ushort, (IntVec, float)> a = new Dictionary<ushort, (IntVec, float)>();
//            if (a.TryGetValue(10, out var x))
//            {

//            }
//        }

//        [UnitTest]
//        public void virtual_generic_method_param_exist()
//        {
//            IVG a = new VirtualGenericMethod();
//            Assert.Equal("$1", a.Foo<float>(1f));
//            Assert.Equal(1, a.Bar<long>(1));

//            VirtualGenericMethodBase b = new VirtualGenericMethod();
//            Assert.Equal("#2", b.Sum<long>(2));
//            Assert.Equal(2f, b.Run<float>(1f, 2f));
//        }

//        [UnitTest]
//        public void virtual_generic_method_param()
//        {
//            IVG a = new VirtualGenericMethod();
//            Assert.Equal("$1", a.Foo<int>(1));
//        }

//        [UnitTest]
//        public void virtual_generic_method_param2()
//        {
//            IVG a = new VirtualGenericMethod();
//            Assert.Equal("$(1,2,3)", a.Foo(new IntVec(1,2,3)));
//        }

//        [UnitTest]
//        public void virtual_generic_method_param3()
//        {
//            VirtualGenericMethodBase a = new VirtualGenericMethod();
//            Assert.Equal("#1", a.Sum(1));
//        }

//        [UnitTest]
//        public void virtual_generic_method_param4()
//        {
//            VirtualGenericMethodBase a = new VirtualGenericMethod();
//            Assert.Equal("#(1,2,3)", a.Sum(new IntVec(1, 2, 3)));
//        }

//        [UnitTest]
//        public void virtual_generic_method_return()
//        {
//            IVG a = new VirtualGenericMethod();
//            Assert.Equal(1, a.Bar<int>(1));
//        }

//        [UnitTest]
//        public void virtual_generic_method_return2()
//        {
//            IVG a = new VirtualGenericMethod();
//            Assert.Equal(new IntVec(1, 2, 3), a.Bar(new IntVec(1, 2, 3)));
//        }

//        [UnitTest]
//        public void virtual_generic_method_return3()
//        {
//            VirtualGenericMethodBase a = new VirtualGenericMethod();
//            Assert.Equal("#1", a.Sum(1));
//        }

//        [UnitTest]
//        public void virtual_generic_method_return4()
//        {
//            VirtualGenericMethodBase a = new VirtualGenericMethod();
//            Assert.Equal(new IntVec(1, 2, 3), a.Run(new IntVec(1,1,1), new IntVec(1, 2, 3)));
//        }

//        [UnitTest]
//        public void virtual_generic_method_return5()
//        {
//            VirtualGenericMethodBase a = new VirtualGenericMethod();
//            Assert.Equal("#1", a.Sum<long>(1));
//        }

//        [UnitTest]
//        public void class_ctor_not_call_managed2native()
//        {
//            var c = new Tuple<int, SkillEquipInfo>(1, default);
//            c.ToString();
//        }

//        [UnitTest]
//        public void valueType_ctor_not_call_managed2native()
//        {
//            var c = new ValueTuple<int, SkillEquipInfo>(1, default);
//            c.ToString();
//        }
//    }
//}
