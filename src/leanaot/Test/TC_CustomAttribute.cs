
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;


namespace Tests.CSharp.CustomeAttrites
{
    public class FT_TypeAttribute : Attribute
    {
        public Type Type { get; set; }

        public object Value { get; set; }
    }

    public class FT_ushortAttribute : Attribute
    {
        public ushort X { get; }

        public FT_ushortAttribute(ushort x)
        {
            X = x;
        }
    }

    public class FT_PrimitiveAttribute : Attribute
    {
        public byte X1 { get; }

        public sbyte X2 { get; }

        public char X3 { get; }

        public short X4 { get; }

        public ushort X5 { get; }

        public int X6 { get; }

        public uint X7 { get; }

        public long X8 { get; }

        public ulong X9 { get; }

        public FT_PrimitiveAttribute(byte x1, sbyte x2, char x3, short x4, ushort x5, int x6, uint x7, long x8, ulong x9)
        {
            X1 = x1;
            X2 = x2;
            X3 = x3;
            X4 = x4;
            X5 = x5;
            X6 = x6;
            X7 = x7;
            X8 = x8;
            X9 = x9;
        }
    }

    public class FT_PrimitiveFieldAttribute : Attribute
    {
        public byte x1;

        public sbyte x2;

        public char x3;

        public short x4;

        public ushort x5;

        public int x6;

        public uint x7;

        public long x8;

        public ulong x9;

        public string x10;

        public FT_PrimitiveFieldAttribute(byte x1, sbyte x2, char x3, short x4, ushort x5, int x6, uint x7, long x8, ulong x9, string x10)
        {
            this.x1 = x1;
            this.x2 = x2;
            this.x3 = x3;
            this.x4 = x4;
            this.x5 = x5;
            this.x6 = x6;
            this.x7 = x7;
            this.x8 = x8;
            this.x9 = x9;
            this.x10 = x10;
        }
    }

    public class FT_EnumAttribute : Attribute
    {
        public FT_EnumAttribute(AOT_Enum_int x)
        {
            X = x;
        }

        public AOT_Enum_int X { get; }
    }

    public class FT_BoxedArrayValueAttribute : Attribute
    {
        public object[] Args { get; }

        public FT_BoxedArrayValueAttribute(params object[] args)
        {
            Args = args;
        }
    }

    public class FT_BoxedValueAttribute : Attribute
    {
        public object Arg { get; }

        public FT_BoxedValueAttribute(object arg)
        {
            Arg = arg;
        }
    }


    public class FT_IntParamsAttribute : Attribute
    {
        public int[] Args { get; }

        public FT_IntParamsAttribute(params int[] args)
        {
            Args = args;
        }
    }

    public class FT_EnumParamsAttribute : Attribute
    {
        public AOT_Enum_int[] Args { get; }

        public FT_EnumParamsAttribute(params AOT_Enum_int[] args)
        {
            Args = args;
        }
    }

    public class FT_TypeParamsAttribute : Attribute
    {
        public Type[] Args { get; }

        public FT_TypeParamsAttribute(params Type[] args)
        {
            Args = args;
        }
    }

    [FT_ushort(1234)]
    internal class TC_CustomAttribute
    {
        [UnitTest]
        [FT_ushort(1122)]
        public void test_ushort_method()
        {
            var method = GetType().GetMethod(nameof(test_ushort_method));
            var attrs = method.GetCustomAttributes(typeof(FT_ushortAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            FT_ushortAttribute attr = (FT_ushortAttribute)attrs[0];
            Assert.Equal(1122, attr.X);
        }

        [UnitTest]
        public void test_ushort_class()
        {
            var attrs = GetType().GetCustomAttributes(typeof(FT_ushortAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            FT_ushortAttribute attr = (FT_ushortAttribute)attrs[0];
            Assert.Equal(1234, attr.X);
        }


        [UnitTest]
        [FT_Primitive(1, -1, 'a', -2, 2, -3, 3, -4, 4)]
        public void test_primitive_types()
        {
            var method = GetType().GetMethod(nameof(test_primitive_types));
            var attrs = method.GetCustomAttributes(typeof(FT_PrimitiveAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            FT_PrimitiveAttribute attr = (FT_PrimitiveAttribute)attrs[0];
            Assert.Equal(1, attr.X1);
            Assert.Equal(-1, attr.X2);
            Assert.Equal('a', attr.X3);
            Assert.Equal(-2, attr.X4);
            Assert.Equal(2, attr.X5);
            Assert.Equal(-3, attr.X6);
            Assert.Equal(3, attr.X7);
            Assert.Equal(-4, attr.X8);
            Assert.Equal(4, attr.X9);
        }

        [UnitTest]
        [FT_PrimitiveField(1, -1, 'a', -2, 2, -3, 3, -4, 4, "abc")]
        public void test_primitive_fields()
        {
            var method = GetType().GetMethod(nameof(test_primitive_fields));
            var attrs = method.GetCustomAttributes(typeof(FT_PrimitiveFieldAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            FT_PrimitiveFieldAttribute attr = (FT_PrimitiveFieldAttribute)attrs[0];
            Assert.Equal(1, attr.x1);
            Assert.Equal(-1, attr.x2);
            Assert.Equal('a', attr.x3);
            Assert.Equal(-2, attr.x4);
            Assert.Equal(2, attr.x5);
            Assert.Equal(-3, attr.x6);
            Assert.Equal(3, attr.x7);
            Assert.Equal(-4, attr.x8);
            Assert.Equal(4, attr.x9);
            Assert.Equal("abc", attr.x10);
        }

        [UnitTest]
        [FT_Enum(AOT_Enum_int.A)]
        public void InterpreterEnumAttriteField()
        {
            var method = GetType().GetMethod(nameof(InterpreterEnumAttriteField));
            var attrs = method.GetCustomAttributes(typeof(FT_EnumAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            FT_EnumAttribute attr = (FT_EnumAttribute)attrs[0];
            Assert.Equal(AOT_Enum_int.A, attr.X);
        }

        [UnitTest]
        [FT_BoxedValue(AOT_Enum_int.A)]
        public void fixedarg_boxed_enum0()
        {
            var method = GetType().GetMethod(nameof(fixedarg_boxed_enum0));
            var attrs = method.GetCustomAttributes(typeof(FT_BoxedValueAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_BoxedValueAttribute)attrs[0];
            Assert.Equal(AOT_Enum_int.A, (AOT_Enum_int)attr.Arg);
        }

        [UnitTest]
        [FT_BoxedArrayValue(AOT_Enum_int.A)]
        public void fixedarg_boxed_enum()
        {
            var method = GetType().GetMethod(nameof(fixedarg_boxed_enum));
            var attrs = method.GetCustomAttributes(typeof(FT_BoxedArrayValueAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_BoxedArrayValueAttribute)attrs[0];
            Assert.Equal(AOT_Enum_int.A, (AOT_Enum_int)attr.Args[0]);
        }

        [UnitTest]
        [FT_BoxedArrayValue(11L)]
        public void fixedarg_boxed_long()
        {
            var method = GetType().GetMethod(nameof(fixedarg_boxed_long));
            var attrs = method.GetCustomAttributes(typeof(FT_BoxedArrayValueAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_BoxedArrayValueAttribute)attrs[0];
            Assert.Equal(11L, (long)attr.Args[0]);
        }

        public struct Vector2d
        {
            public int x;
            public int y;
        }

        [UnitTest]
        [FT_BoxedArrayValue(typeof(Vector2d))]
        public void fixedarg_type()
        {
            var method = GetType().GetMethod(nameof(fixedarg_type));
            var attrs = method.GetCustomAttributes(typeof(FT_BoxedArrayValueAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_BoxedArrayValueAttribute)attrs[0];
            Assert.Equal(typeof(Vector2d), (Type)attr.Args[0]);
        }

        [UnitTest]
        [FT_IntParams(11, 1214)]
        public void CtorIntParams()
        {
            var method = GetType().GetMethod(nameof(CtorIntParams));
            var attrs = method.GetCustomAttributes(typeof(FT_IntParamsAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_IntParamsAttribute)attrs[0];
            Assert.Equal(11, attr.Args[0]);
            Assert.Equal(1214, attr.Args[1]);
        }

        [UnitTest]
        [FT_EnumParams(AOT_Enum_int.A, AOT_Enum_int.D)]
        public void CtorEnumParams()
        {
            var method = GetType().GetMethod(nameof(CtorEnumParams));
            var attrs = method.GetCustomAttributes(typeof(FT_EnumParamsAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_EnumParamsAttribute)attrs[0];
            Assert.Equal(AOT_Enum_int.A, attr.Args[0]);
            Assert.Equal(AOT_Enum_int.D, attr.Args[1]);
        }

        [UnitTest]
        [EnumField(AOT_Enum_int.A)]
        public void AOTEnumAttriteField()
        {
            var method = GetType().GetMethod(nameof(AOTEnumAttriteField));
            var attrs = method.GetCustomAttributes(typeof(EnumFieldAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (EnumFieldAttribute)attrs[0];
            Assert.Equal(AOT_Enum_int.A, attr.X);
        }

        [UnitTest]
        public void is_define_ushort_class()
        {
            var ret = GetType().IsDefined(typeof(FT_ushortAttribute), false);
            Assert.True(ret);

            var ret2 = GetType().IsDefined(typeof(FT_EnumAttribute), false);
            Assert.False(ret2);
        }

        [UnitTest]
        [FT_Type(Type = typeof(Vector3), Value = typeof(FT_ushortAttribute))]
        public void NamedArgSystemType()
        {
            var method = GetType().GetMethod(nameof(NamedArgSystemType));
            Assert.NotNull(method);
            var attrs = method.GetCustomAttributes(typeof(FT_TypeAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_TypeAttribute)attrs[0];
            Assert.Equal(typeof(Vector3), attr.Type);
            Assert.Equal(typeof(FT_ushortAttribute), attr.Value);
        }

        [UnitTest]
        [FT_ushort(1122)]
        [FT_Enum(AOT_Enum_int.A)]
        public void test_ushort_method_CustomAttributeData()
        {
            var method = GetType().GetMethod(nameof(test_ushort_method_CustomAttributeData));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(3, attrs.Count);
            CustomAttributeData d1 = attrs[0];
            Assert.Equal(typeof(UnitTestAttribute), d1.AttributeType);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(FT_ushortAttribute), d2.AttributeType);

            CustomAttributeData d3 = attrs[2];
            Assert.Equal(typeof(FT_EnumAttribute), d3.AttributeType);
        }

        [UnitTest]
        [FT_TypeParams(typeof(object), typeof(Vector3), typeof(FT_ushortAttribute))]
        public void test_TypeParams()
        {
            var method = GetType().GetMethod(nameof(test_TypeParams));
            var attrs = method.GetCustomAttributes(typeof(FT_TypeParamsAttribute), false);
            Assert.NotNull(attrs);
            Assert.Equal(1, attrs.Length);
            var attr = (FT_TypeParamsAttribute)attrs[0];
            Assert.Equal(typeof(object), attr.Args[0]);
            Assert.Equal(typeof(Vector3), attr.Args[1]);
            Assert.Equal(typeof(FT_ushortAttribute), attr.Args[2]);
        }

        public class FT_CtorAttribute : Attribute
        {
            public int X { get; }

            public FT_CtorAttribute(int x)
            {
                X = x;
            }
        }

        [UnitTest]
        [FT_Ctor(1)]
        public void test_CustomAttributeData_CtorArg()
        {
            // support CustomeAttributeData.NamedArguments since 2021

            var method = GetType().GetMethod(nameof(test_CustomAttributeData_CtorArg));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(2, attrs.Count);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(FT_CtorAttribute), d2.AttributeType);
            var cargs = d2.ConstructorArguments;
            Assert.Equal(1, cargs.Count);
            Assert.Equal(1, cargs[0].Value);
        }

        public class NamedArgAttribute : Attribute
        {
            public int X { get; set; }

            public int y;
        }

        public class NamedArgChildAttribute : NamedArgAttribute
        {

        }

        [UnitTest]
        [NamedArg(X = 1)]
        public void test_CustomAttributeData_NamedArg_Property()
        {
            // support CustomeAttributeData.NamedArguments since 2021
            var method = GetType().GetMethod(nameof(test_CustomAttributeData_NamedArg_Property));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(2, attrs.Count);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(NamedArgAttribute), d2.AttributeType);
            var cargs = d2.NamedArguments;
            Assert.Equal(1, cargs.Count);
            Assert.Equal("X", cargs[0].MemberName);
            Assert.Equal(1, cargs[0].TypedValue.Value);
        }

        [UnitTest]
        [NamedArgChild(X = 1)]
        public void test_CustomAttributeData_NamedArg_Property_Parent()
        {
            // support CustomeAttributeData.NamedArguments since 2021
            var method = GetType().GetMethod(nameof(test_CustomAttributeData_NamedArg_Property_Parent));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(2, attrs.Count);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(NamedArgChildAttribute), d2.AttributeType);
            var cargs = d2.NamedArguments;
            Assert.Equal(1, cargs.Count);
            Assert.Equal("X", cargs[0].MemberName);
            Assert.Equal(1, cargs[0].TypedValue.Value);
        }

        [UnitTest]
        [NamedArg(y = 2)]
        public void test_CustomAttributeData_NamedArg_Field()
        {
            // support CustomeAttributeData.NamedArguments since 2021
            var method = GetType().GetMethod(nameof(test_CustomAttributeData_NamedArg_Field));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(2, attrs.Count);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(NamedArgAttribute), d2.AttributeType);
            var cargs = d2.NamedArguments;
            Assert.Equal(1, cargs.Count);
            Assert.Equal("y", cargs[0].MemberName);
            Assert.Equal(2, cargs[0].TypedValue.Value);
        }

        [UnitTest]
        [NamedArgChild(y = 2)]
        public void test_CustomAttributeData_NamedArg_Field_Parent()
        {
            var method = GetType().GetMethod(nameof(test_CustomAttributeData_NamedArg_Field_Parent));
            var attrs = method.GetCustomAttributesData();
            Assert.NotNull(attrs);
            Assert.Equal(2, attrs.Count);

            CustomAttributeData d2 = attrs[1];
            Assert.Equal(typeof(NamedArgChildAttribute), d2.AttributeType);
            var cargs = d2.NamedArguments;
            Assert.Equal(1, cargs.Count);
            Assert.Equal("y", cargs[0].MemberName);
            Assert.Equal(2, cargs[0].TypedValue.Value);
        }

        [UnitTest]
        public void assembly_CustomAttribute()
        {
            var ass = this.GetType().Assembly;
            var copyRight = ass.GetCustomAttribute<AssemblyCopyrightAttribute>();
            Assert.NotNull(copyRight);
            Assert.True(copyRight.Copyright.StartsWith("Copyright"));
        }

        public class ParamAttribute : System.Attribute
        {
            public int Value { get; }

            public ParamAttribute(int value)
            {
                Value = value;
            }
        }

        [return: Param(10)]
        public static int MPCA([Param(100)] int x, [Param(200)] int y)
        {
            return 0;
        }

        [UnitTest]
        public void MethodParamCustomAttribute()
        {
            MethodInfo method = GetType().GetMethod("MPCA", BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public);

            var args = method.GetParameters();
            var p1 = args[0].GetCustomAttribute<ParamAttribute>();
            Assert.Equal(100, p1.Value);
            var p2 = args[1].GetCustomAttribute<ParamAttribute>();
            Assert.Equal(200, p2.Value);
        }

        [UnitTest]
        public void MethodReturnTypeCustomAttribute()
        {
            MethodInfo method = GetType().GetMethod("MPCA", BindingFlags.Instance | BindingFlags.Static | BindingFlags.Public);

            var rp = method.ReturnParameter;
            var pa = rp.GetCustomAttribute<ParamAttribute>();
            if (pa != null)
            {
                Assert.Equal(10, pa.Value);
            }
            else
            {
                Assert.Null(pa);
            }
        }
    }
}
