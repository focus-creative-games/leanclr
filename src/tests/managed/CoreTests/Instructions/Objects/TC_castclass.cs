using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Tests.Instruments.Objs
{
    internal class TC_castclass : GeneralTestCaseBase
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

        public static T CastClass<T>(object o)
        {
            return (T)o;
        }

        [UnitTest]
        public void object_1()
        {
            object o = "abc";
            string s = (string)o;
            Assert.Equal("abc", s);
        }

        //[UnitTest]
        //public void object_2()
        //{
        //    object o = 1;
        //    Assert.ExpectException<InvalidCastException>();
        //    string s = (string)o;
        //    Assert.Fail();
        //}

        [UnitTest]
        public void int_1()
        {
            object o = 1;
            int x = CastClass<int>(o);
            Assert.Equal(1, x);
        }

        [UnitTest]
        public void nullable_int_from_boxed_int()
        {
            object o = 7;
            int? x = CastClass<int?>(o);
            Assert.True(x.HasValue);
            Assert.Equal(7, x.Value);
        }

        [UnitTest]
        public void nullable_int_from_null_object()
        {
            object o = null;
            int? x = CastClass<int?>(o);
            Assert.False(x.HasValue);
        }

        [UnitTest]
        public void nullable_int_invalid_from_long()
        {
            object o = 1L;
            Assert.ExpectException<InvalidCastException>(() =>
            {
                int? x = CastClass<int?>(o);
            });
        }

        [UnitTest]
        public void enum_cast_same_type()
        {
            object o = SampleEnum.B;
            SampleEnum e = CastClass<SampleEnum>(o);
            Assert.Equal(SampleEnum.B, e);
        }

        [UnitTest]
        public void enum_cast_invalid_from_int()
        {
            object o = 1;
            SampleEnum e = CastClass<SampleEnum>(o);
            e.ToString();
        }

        [UnitTest]
        public void enum_cast_invalid_to_other_enum()
        {
            object o = SampleEnum.A;
            Assert.ExpectException<InvalidCastException>(() =>
            {
                ByteBackedEnum e = CastClass<ByteBackedEnum>(o);
            });
        }

        [UnitTest]
        public void enum_cast_invalid_from_enum_to_int()
        {
            object o = SampleEnum.A;
            int x = CastClass<int>(o);
            x.ToString();
        }

        [UnitTest]
        public void byte_backed_enum_cast_same_type()
        {
            object o = ByteBackedEnum.Y;
            ByteBackedEnum e = CastClass<ByteBackedEnum>(o);
            Assert.Equal(ByteBackedEnum.Y, e);
        }

        [UnitTest]
        public void nullable_enum_has_value_to_enum()
        {
            SampleEnum? n = SampleEnum.A;
            object o = n;
            SampleEnum e = CastClass<SampleEnum>(o);
            Assert.Equal(SampleEnum.A, e);
        }

        [UnitTest]
        public void nullable_enum_has_value_to_nullable_enum()
        {
            SampleEnum? n = SampleEnum.B;
            object o = n;
            SampleEnum? x = CastClass<SampleEnum?>(o);
            Assert.True(x.HasValue);
            Assert.Equal(SampleEnum.B, x.Value);
        }

        [UnitTest]
        public void nullable_enum_null_to_nullable_enum()
        {
            SampleEnum? n = null;
            object o = n;
            SampleEnum? x = CastClass<SampleEnum?>(o);
            Assert.False(x.HasValue);
        }

        [UnitTest]
        public void nullable_bool_has_value()
        {
            bool? b = true;
            object o = b;
            bool? x = CastClass<bool?>(o);
            Assert.True(x.HasValue);
            Assert.True(x.Value);
        }

        [UnitTest]
        public void nullable_bool_null()
        {
            bool? b = null;
            object o = b;
            bool? x = CastClass<bool?>(o);
            Assert.False(x.HasValue);
        }

        [UnitTest]
        public void nullable_double_has_value()
        {
            double? d = 2.5;
            object o = d;
            double? x = CastClass<double?>(o);
            Assert.True(x.HasValue);
            Assert.Equal(2.5, x.Value);
        }

        [UnitTest]
        public void nullable_struct_has_value_to_struct()
        {
            SampleStruct? s = new SampleStruct { V = 9 };
            object o = s;
            SampleStruct x = CastClass<SampleStruct>(o);
            Assert.Equal(9, x.V);
        }

        [UnitTest]
        public void nullable_struct_has_value_to_nullable_struct()
        {
            SampleStruct? s = new SampleStruct { V = 4 };
            object o = s;
            SampleStruct? x = CastClass<SampleStruct?>(o);
            Assert.True(x.HasValue);
            Assert.Equal(4, x.Value.V);
        }

        [UnitTest]
        public void nullable_struct_null()
        {
            SampleStruct? s = null;
            object o = s;
            SampleStruct? x = CastClass<SampleStruct?>(o);
            Assert.False(x.HasValue);
        }

        [UnitTest]
        public void combination_int_not_enum_cast()
        {
            object o = 42;
            int x = CastClass<int>(o);
            Assert.Equal(42, x);
            SampleEnum e = CastClass<SampleEnum>(o);
            e.ToString();
        }

        [UnitTest]
        public void combination_underlying_int_not_enum_cast()
        {
            object o = (int)SampleEnum.A;
            Assert.Equal(1, CastClass<int>(o));
            SampleEnum e = CastClass<SampleEnum>(o);
            e.ToString();
        }

        [UnitTest]
        public void combination_enum_to_nullable_enum_roundtrip()
        {
            object o = SampleEnum.B;
            SampleEnum e = CastClass<SampleEnum>(o);
            Assert.Equal(SampleEnum.B, e);
            SampleEnum? n = CastClass<SampleEnum?>(o);
            Assert.True(n.HasValue);
            Assert.Equal(SampleEnum.B, n.Value);
        }

        [UnitTest]
        public void combination_nullable_enum_not_other_nullable_enum()
        {
            SampleEnum? n = SampleEnum.A;
            object o = n;
            SampleEnum? x = CastClass<SampleEnum?>(o);
            Assert.True(x.HasValue);
            Assert.ExpectException<InvalidCastException>(() =>
            {
                ByteBackedEnum? y = CastClass<ByteBackedEnum?>(o);
            });
        }
    }
}
