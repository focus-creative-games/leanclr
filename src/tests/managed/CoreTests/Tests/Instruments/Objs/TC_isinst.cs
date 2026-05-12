using test;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Objs
{
    internal class TC_isinst : GeneralTestCaseBase
    {
        private enum SampleEnum
        {
            A = 1,
            B = 2,
        }

        private enum ByteBackedEnum : byte
        {
            X = 0,
            Y = 1,
        }

        private struct SampleStruct
        {
            public int V;
        }

        [UnitTest]
        public void int_1()
        {
            object o = 1;
            Assert.True(o is int);
        }

        [UnitTest]
        public void int_2()
        {
            object o = 1;
            Assert.False(o is long);
        }

        [UnitTest]
        public void int_3()
        {
            object o = null;
            Assert.False(o is int);
        }

        [UnitTest]
        public void int_4()
        {
            object o = 1;
            Assert.False(o is string);
        }

        class A
        {

        }

        class B : A, IEnumerable, IEnumerable<int>
        {
            public IEnumerator<int> GetEnumerator()
            {
                throw new NotImplementedException();
            }

            IEnumerator IEnumerable.GetEnumerator()
            {
                throw new NotImplementedException();
            }
        }

        [UnitTest]
        public void class_1()
        {
            object o = new B();
            Assert.True(o is A);
        }

        [UnitTest]
        public void class_2()
        {
            object o = new A();
            Assert.False(o is B);
        }

        [UnitTest]
        public void class_3()
        {
            object o = new B();
            Assert.True(o is object);
        }

        [UnitTest]
        public void class_4()
        {
            object o = new B();
            Assert.False(o is string);
        }

        [UnitTest]
        public void class_5()
        {
            object o = new B();
            Assert.True(o is IEnumerable);
        }

        [UnitTest]
        public void class_6()
        {
            object o = new B();
            Assert.True(o is IEnumerable<int>);
        }

        [UnitTest]
        public void class_7()
        {
            object o = new B();
            Assert.False(o is IEnumerable<string>);
        }

        [UnitTest]
        public void nullable_int_1()
        {
            object o = 1;
            Assert.True(o is int?);
        }

        [UnitTest]
        public void nullable_int_2()
        {
            object o = 1;
            Assert.False(o is long?);
        }

        [UnitTest]
        public void nullable_int_3()
        {
            object o = null;
            Assert.False(o is int?);
        }

        [UnitTest]
        public void enum_boxed_same_type()
        {
            object o = SampleEnum.A;
            Assert.True(o is SampleEnum);
        }

        [UnitTest]
        public void enum_boxed_other_enum()
        {
            object o = SampleEnum.A;
            Assert.False(o is ByteBackedEnum);
        }

        [UnitTest]
        public void enum_boxed_not_underlying_int()
        {
            object o = SampleEnum.A;
            Assert.False(o is int);
        }

        [UnitTest]
        public void enum_boxed_is_enum()
        {
            object o = SampleEnum.B;
            Assert.True(o is Enum);
        }

        [UnitTest]
        public void enum_byte_backed_same_type()
        {
            object o = ByteBackedEnum.Y;
            Assert.True(o is ByteBackedEnum);
            Assert.False(o is SampleEnum);
        }

        [UnitTest]
        public void nullable_enum_has_value_same_enum_nullable()
        {
            SampleEnum? n = SampleEnum.B;
            object o = n;
            Assert.True(o is SampleEnum?);
        }

        [UnitTest]
        public void nullable_enum_has_value_same_enum_non_nullable()
        {
            SampleEnum? n = SampleEnum.A;
            object o = n;
            Assert.True(o is SampleEnum);
        }

        [UnitTest]
        public void nullable_enum_no_value_object_is_null()
        {
            SampleEnum? n = null;
            object o = n;
            Assert.Null(o);
        }

        [UnitTest]
        public void nullable_enum_no_value_is_not_nullable_enum()
        {
            SampleEnum? n = null;
            object o = n;
            Assert.False(o is SampleEnum?);
        }

        [UnitTest]
        public void nullable_bool_has_value()
        {
            bool? b = true;
            object o = b;
            Assert.True(o is bool?);
            Assert.True(o is bool);
        }

        [UnitTest]
        public void nullable_bool_no_value_object_null()
        {
            bool? b = null;
            object o = b;
            Assert.Null(o);
            Assert.False(o is bool?);
        }

        [UnitTest]
        public void nullable_double_has_value()
        {
            double? d = 3.14;
            object o = d;
            Assert.True(o is double?);
            Assert.True(o is double);
        }

        [UnitTest]
        public void nullable_struct_has_value()
        {
            SampleStruct? s = new SampleStruct { V = 7 };
            object o = s;
            Assert.True(o is SampleStruct?);
            Assert.True(o is SampleStruct);
        }

        [UnitTest]
        public void nullable_struct_no_value()
        {
            SampleStruct? s = null;
            object o = s;
            Assert.Null(o);
            Assert.False(o is SampleStruct?);
        }

        [UnitTest]
        public void combination_int_boxed_not_enum()
        {
            object o = 42;
            Assert.True(o is int);
            Assert.True(o is int?);
            Assert.False(o is SampleEnum);
            Assert.False(o is SampleEnum?);
        }

        [UnitTest]
        public void combination_underlying_int_boxed_not_enum()
        {
            object o = (int)SampleEnum.A;
            Assert.True(o is int);
            Assert.False(o is SampleEnum);
        }

        [UnitTest]
        public void combination_enum_boxed_matches_nullable_enum()
        {
            object o = SampleEnum.B;
            Assert.True(o is SampleEnum);
            Assert.True(o is SampleEnum?);
        }

        [UnitTest]
        public void combination_long_boxed_not_int_nullable()
        {
            object o = 1L;
            Assert.True(o is long);
            Assert.True(o is long?);
            Assert.False(o is int?);
        }

        [UnitTest]
        public void combination_nullable_enum_other_nullable_enum()
        {
            SampleEnum? n = SampleEnum.A;
            object o = n;
            Assert.True(o is SampleEnum?);
            Assert.False(o is ByteBackedEnum?);
        }
    }
}
