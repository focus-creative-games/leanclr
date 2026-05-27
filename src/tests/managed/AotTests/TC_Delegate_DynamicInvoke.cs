
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace Tests.CSharp.Delegates
{
    public struct FT_AOT_ValueType
    {
        public int x;
        public float y;
        public string z;

        public void Run(int a)
        {
            x += a;
        }

        public int Show(int b)
        {
            return this.x + b;
        }

        public sbyte Foo(int b)
        {
            return (sbyte)(x + b);
        }
    }

    public delegate void DelAOTRun(ref FT_AOT_ValueType a, int b);
    public delegate int DelAOTShow(ref FT_AOT_ValueType a, int b);
    public delegate sbyte DelAOTFoo(ref FT_AOT_ValueType a, int b);



    public delegate void DelAOTRun2(int b);
    public delegate int DelAOTShow2(int b);
    public delegate sbyte DelAOTFoo2(int b);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate IntPtr CppBattleEngineReadFileEvent(IntPtr fileName);


    public class FT_AOT_Class
    {
        public int x;
        public float y;
        public string z;

        public void Run(int a)
        {
            x += a;
        }

        public int Show(int b)
        {
            return this.x + b;
        }

        public sbyte Foo(int b)
        {
            return (sbyte)(x + b);
        }

        public static void Run2(FT_AOT_Class s, int b)
        {
            s.x += b;
        }

        public static void Run3(FT_AOT_Class s, int b)
        {

        }

        public static int Show2(FT_AOT_Class s, int b)
        {
            return s.x + b;
        }

        public static sbyte Foo2(FT_AOT_Class s, int b)
        {
            return (sbyte)(s.x + b);
        }
    }

    public delegate void ClassRun1(int b);
    public delegate int ClassShow1(int b);
    public delegate sbyte ClassFoo1(int b);
    public delegate void AOTClassRun1(int b);
    public delegate int AOTClassShow1(int b);
    public delegate sbyte AOTClassFoo1(int b);

    public delegate void ClassDel1(ValueTypeSize9 x);

    public delegate void ClassRun0(FT_Class a);
    public delegate void ClassRun2(FT_Class a, int b);
    public delegate int ClassShow2(FT_Class a, int b);
    public delegate sbyte ClassFoo2(FT_Class a, int b);


    public delegate void AOTClassRun2(FT_AOT_Class a, int b);
    public delegate int AOTClassShow2(FT_AOT_Class a, int b);
    public delegate sbyte AOTClassFoo2(FT_AOT_Class a, int b);

    public class TC_Delegate_DynamicInvoke
    {

        /// <summary>
        /// IL2CPP BUG. 不支持由instance method创建的open delegate上调用DynamicInvoke
        /// </summary>
        [UnitTest]
        public void void_class_instance_open_interp()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run");
            var del = (ClassRun2)Delegate.CreateDelegate(typeof(ClassRun2), null, m);

            var invoke = typeof(ClassRun2).GetMethod("Invoke");
            invoke.Invoke(del, new object[] { b, 4 });
            Assert.Equal(5, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { b, 1 });
            Assert.Equal(7, b.x);

            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del, new object[] { null, 4 });
            });
        }

        /// <summary>
        /// IL2CPP BUG. 不支持由instance method创建的open delegate上调用DynamicInvoke
        /// </summary>
        [UnitTest]
        public void void_class_instance_open_interp2()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run0");
            var del = (ClassRun0)Delegate.CreateDelegate(typeof(ClassRun0), null, m);

            var invoke = typeof(ClassRun0).GetMethod("Invoke");
            invoke.Invoke(del, new object[] { b });
            Assert.Equal(2, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { b });
            Assert.Equal(4, b.x);

            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del, new object[] { null, 4 });
            });
        }


        [UnitTest]
        public void void_class_instance_close_interp()
        {
            // <= 2020, raise NullReferenceException
            // >= 2021, raise ArgumentException
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run");
            var del = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), b, m);

            var invoke = typeof(ClassRun1).GetMethod("Invoke");

            invoke.Invoke(del, new object[] { 4 });
            Assert.Equal(5, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { 1 });
            Assert.Equal(7, b.x);

            var del2 = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), null, m);
            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del2, new object[] { 1 });
            });
        }

        [UnitTest]
        public void void_class_static_open_interp()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run2");
            var invoke = typeof(ClassRun2).GetMethod("Invoke");

            var del = (ClassRun2)Delegate.CreateDelegate(typeof(ClassRun2), null, m);
            invoke.Invoke(del, new object[] { b, 4 });
            Assert.Equal(5, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { b, 1 });
            Assert.Equal(7, b.x);

            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del, new object[] { null, 1 });
            });
        }

        [UnitTest]
        public void void_class_static_close_interp()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run2");
            var invoke = typeof(ClassRun1).GetMethod("Invoke");
            var del = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), b, m);

            invoke.Invoke(del, new object[] { 4 });
            Assert.Equal(5, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { 1 });
            Assert.Equal(7, b.x);

            var del2 = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), null, m);
            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del2, new object[] { 4 });
            });
        }

        [UnitTest]
        public void void_valuetype_instance_open_interp()
        {
            /*
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Run");
            var invoke = typeof(ValueTypeRun).GetMethod("Invoke");
            var del = (ValueTypeRun)Delegate.CreateDelegate(typeof(ValueTypeRun), null, m);

            object c = b;
            invoke.Invoke(del, new object[] { c, 1 });
            // mono BUG!!! mono 会在此处断言失败, get value 1。
            // 但il2cpp却是正确的!
            Assert.Equal(2, ((FT_ValueType)c).x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { c, 1 });
            Assert.Equal(4, ((FT_ValueType)c).x);
            */
        }

        [UnitTest]
        public void void_valuetype_instance_close_interp()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Run");
            var invoke = typeof(ClassRun1).GetMethod("Invoke");
            object c = b;
            var del = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), c, m);
            invoke.Invoke(del, new object[] { 1 });
            Assert.Equal(2, ((FT_ValueType)c).x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { 1 });
            Assert.Equal(4, ((FT_ValueType)c).x);
        }

        [UnitTest]
        public void void_class_instance_close_aot()
        {
            // <= 2020, raise NullReferenceException
            // >= 2021, raise ArgumentException

            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Run");
            var invoke = typeof(ClassRun1).GetMethod("Invoke");
            var del = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), b, m);
            invoke.Invoke(del, new object[] { 4 });
            Assert.Equal(5, b.x);

            var dd = del + del;
            invoke.Invoke(dd, new object[] { 1 });
            Assert.Equal(7, b.x);

            var del2 = (ClassRun1)Delegate.CreateDelegate(typeof(ClassRun1), null, m);
            Assert.ExpectException<Exception>(() =>
            {
                invoke.Invoke(del2, new object[] { 4 });
            });
        }


    }
}
