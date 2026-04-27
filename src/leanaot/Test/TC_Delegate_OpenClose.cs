using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Tests.CSharp.Delegates
{
    class A
    {
        public virtual int GetX()
        {
            return 0;
        }
    }

    class B : A
    {
        public override int GetX()
        {
            return 1;
        }

        public Func<int> CreateBaseGetX()
        {
            return base.GetX;
        }
    }

    public struct FT_ValueType
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

        public int Foo2()
        {
            return x;
        }
    }

    public delegate void ValueTypeRun(ref FT_ValueType a, int b);
    public delegate int ValueTypeShow(ref FT_ValueType a, int b);
    public delegate sbyte ValueTypeFoo(ref FT_ValueType a, int b);


    public class FT_Class
    {
        public int x;
        public float y;
        public string z;

        public void Run0()
        {
            x += 1;
        }

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

        public static void Run2(FT_Class s, int b)
        {
            s.x += b;
        }

        public static void Run3(FT_Class s, int b)
        {

        }

        public static int Show2(FT_Class s, int b)
        {
            return s.x + b;
        }

        public static sbyte Foo2(FT_Class s, int b)
        {
            return (sbyte)(s.x + b);
        }

        public void Add(ValueTypeSize9 x)
        {
            this.x += x.x1;
        }
    }

    internal class TC_Delegate_OpenClose
    {
        // CallDelegate_void

        /// <summary>
        /// delegate调用所保存的函数时，不应该调用函数相应的虚函数版本
        /// </summary>
        [UnitTest]
        public void NotVirtualDelegateInvoke()
        {
            var b = new B();
            var d = b.CreateBaseGetX();
            var x = d();
            Assert.Equal(0, x);
        }
        [UnitTest]
        public void NotVirtualDelegateInvokeCallByAOT()
        {
            var b = new B();
            var d = b.CreateBaseGetX();
            var x = CallDelegateFuncs.Invoke(d);
            Assert.Equal(0, x);
        }

        [UnitTest]
        public void void_class_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Run");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            del(b, 4);
            Assert.Equal(5, b.x);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                del(null, 4);
            });
        }

        [UnitTest]
        public void void_class_aot_open_invoke_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Run");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            CallDelegateFuncs.Invoke(del, b, 4);
            Assert.Equal(5, b.x);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                del(null, 4);
            });
        }

        [UnitTest]
        public void void_class_aot_close()
        {
            // <= 2020, raise NullReferenceException
            // >= 2021, raise ArgumentException
            Assert.ExpectException<Exception>(() =>
            {
                var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
                var m = typeof(FT_AOT_Class).GetMethod("Run");
                var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
                del(4);
                Assert.Equal(5, b.x);

                var dd = del + del;
                dd(1);
                Assert.Equal(7, b.x);

                var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);

                del2(5);
            });
        }

        [UnitTest]
        public void void_class_aot_close_call_by_aot()
        {
            // <= 2020, raise NullReferenceException
            // >= 2021, raise ArgumentException
            Assert.ExpectException<Exception>(() =>
            {
                var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
                var m = typeof(FT_AOT_Class).GetMethod("Run");
                var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
                CallDelegateFuncs.Invoke(del, 4);
                Assert.Equal(5, b.x);

                var dd = del + del;
                CallDelegateFuncs.Invoke(dd, 1);
                Assert.Equal(7, b.x);

                var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);

                del2(5);
            });
        }

        [UnitTest]
        public void void_class_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run");
            var del = (Action<FT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_Class, int>), null, m);
            del(b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(b, 1);
            Assert.Equal(7, b.x);

            Assert.ExpectException<NullReferenceException>(() =>
            {
                del(null, 4);
            });
        }

        [UnitTest]
        public void void_class_intp_close()
        {
            // <= 2020, raise NullReferenceException
            // >= 2021, raise ArgumentException
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            del(4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(1);
            Assert.Equal(7, b.x);

            var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                del2(5);
            });
        }

        [UnitTest]
        public void void_class_static_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Run2");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            del(b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(b, 1);
            Assert.Equal(7, b.x);
        }


        [UnitTest]
        public void void_class_static_aot_open_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Run2");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            CallDelegateFuncs.Invoke(del, b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            CallDelegateFuncs.Invoke(dd, b, 1);
            Assert.Equal(7, b.x);
        }

        [UnitTest]
        public void void_class_static_aot_open_null_this()
        {
            var m = typeof(FT_AOT_Class).GetMethod("Run3");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            del(null, 4);
        }

        [UnitTest]
        public void void_class_static_aot_open_null_this_call_by_aot()
        {
            var m = typeof(FT_AOT_Class).GetMethod("Run3");
            var del = (Action<FT_AOT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_AOT_Class, int>), null, m);
            CallDelegateFuncs.Invoke(del, null, 4);
        }

        [UnitTest]
        public void void_class_static_aot_close()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Run2");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            del(4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(1);
            Assert.Equal(7, b.x);

            var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                del2(4);
            });
            // 这个测试用例在2019 WebGL平台下会执行到这，导致测试用例失败。
            // 原因是WebGL平台的生成的代码有bug, FT_AOT_Class::Run2中 取对象成员时未检查 是否为NULL。
            // 但其他平台是OK的。感觉很莫名其妙。
        }

        [UnitTest]
        public void void_class_static_aot_close_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Run2");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            CallDelegateFuncs.Invoke(del, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            CallDelegateFuncs.Invoke(dd, 1);
            Assert.Equal(7, b.x);

            var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                CallDelegateFuncs.Invoke(del2, 4);
            });
            // 这个测试用例在2019 WebGL平台下会执行到这，导致测试用例失败。
            // 原因是WebGL平台的生成的代码有bug, FT_AOT_Class::Run2中 取对象成员时未检查 是否为NULL。
            // 但其他平台是OK的。感觉很莫名其妙。
        }

        [UnitTest]
        public void void_class_static_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run2");
            var del = (Action<FT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_Class, int>), null, m);
            del(b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(b, 1);
            Assert.Equal(7, b.x);

            Assert.ExpectException<NullReferenceException>(() =>
            {
                del(null, 4);
            });
        }

        [UnitTest]
        public void void_class_static_intp_open_null_this()
        {
            var m = typeof(FT_Class).GetMethod("Run3");
            var del = (Action<FT_Class, int>)Delegate.CreateDelegate(typeof(Action<FT_Class, int>), null, m);
            del(null, 4);
        }

        [UnitTest]
        public void void_class_static_intp_close()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Run2");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            del(4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(1);
            Assert.Equal(7, b.x);

            var del2 = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), null, m);
            Assert.ExpectException<NullReferenceException>(() =>
            {
                del2(4);
            });
        }

        [UnitTest]
        public void void_valuetype_aot_open()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Run");
            var del = (DelAOTRun)Delegate.CreateDelegate(typeof(DelAOTRun), null, m);
            del(ref b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(ref b, 1);
            Assert.Equal(7, b.x);
        }

        /// <summary>
        /// IL2CPP_BUG 2019 对于open delegate并且是值类型会产生重复调用，因此暂时关闭这个测试用例
        /// </summary>
        /*
        [UnitTest]
        public void void_valuetype_aot_open_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Run");
            var del = (DelAOTRun)Delegate.CreateDelegate(typeof(DelAOTRun), null, m);
            CallDelegateFuncs.Invoke(del, ref b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            CallDelegateFuncs.Invoke(dd, ref b, 1);
            Assert.Equal(7, b.x);
        }
        */

        [UnitTest]
        public void void_valuetype_aot_close()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Run");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            del(4);
            Assert.Equal(5, ((FT_AOT_ValueType)del.Target).x);

            var dd = del + del;
            dd(1);
            Assert.Equal(7, ((FT_AOT_ValueType)del.Target).x);
        }

        [UnitTest]
        public void void_valuetype_aot_close_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Run");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            CallDelegateFuncs.Invoke(del, 4);
            Assert.Equal(5, ((FT_AOT_ValueType)del.Target).x);

            var dd = del + del;
            CallDelegateFuncs.Invoke(dd, 1);
            Assert.Equal(7, ((FT_AOT_ValueType)del.Target).x);
        }

        [UnitTest]
        public void void_valuetype_intp_open()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Run");
            var del = (ValueTypeRun)Delegate.CreateDelegate(typeof(ValueTypeRun), null, m);
            del(ref b, 4);
            Assert.Equal(5, b.x);

            var dd = del + del;
            dd(ref b, 1);
            Assert.Equal(7, b.x);
        }

        [UnitTest]
        public void void_valuetype_intp_close()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Run");
            var del = (Action<int>)Delegate.CreateDelegate(typeof(Action<int>), b, m);
            del(4);
            Assert.Equal(5, ((FT_ValueType)del.Target).x);

            var dd = del + del;
            dd(1);
            Assert.Equal(7, ((FT_ValueType)del.Target).x);
        }

        // CallDelegate_ret

        [UnitTest]
        public void ret_class_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Show");
            var del = (Func<FT_AOT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, int>), null, m);
            Assert.Equal(5, del(b, 4));

            var dd = del + del;
            Assert.Equal(5, dd(b, 4));
        }


        [UnitTest]
        public void ret_class_aot_open_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Show");
            var del = (Func<FT_AOT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, int>), null, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, b, 4));

            var dd = del + del;
            Assert.Equal(5, dd(b, 4));
        }

        [UnitTest]
        public void ret_class_aot_close()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));

            var dd = del + del;
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void ret_class_aot_close_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));

            var dd = del + del;
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, 4));
        }

        [UnitTest]
        public void ret_class_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Show");
            var del = (Func<FT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_Class, int, int>), null, m);
            Assert.Equal(5, del(b, 4));
        }

        [UnitTest]
        public void ret_class_intp_close()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));
        }

        [UnitTest]
        public void ret_class_static_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Show2");
            var del = (Func<FT_AOT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, int>), null, m);
            Assert.Equal(5, del(b, 4));
        }

        [UnitTest]
        public void ret_class_static_aot_open_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Show2");
            var del = (Func<FT_AOT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, int>), null, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, b, 4));
        }

        [UnitTest]
        public void ret_class_static_aot_close()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Show2");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));
        }

        [UnitTest]
        public void ret_class_static_aot_close_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Show2");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));
        }

        [UnitTest]
        public void ret_class_static_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Show2");
            var del = (Func<FT_Class, int, int>)Delegate.CreateDelegate(typeof(Func<FT_Class, int, int>), null, m);
            Assert.Equal(5, del(b, 4));
        }

        [UnitTest]
        public void ret_class_static_intp_close()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Show2");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));
        }

        [UnitTest]
        public void ret_valuetype_aot_open()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Show");
            var del = (DelAOTShow)Delegate.CreateDelegate(typeof(DelAOTShow), null, m);
            Assert.Equal(5, del(ref b, 4));
        }

        [UnitTest]
        public void ret_valuetype_aot_open_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Show");
            var del = (DelAOTShow)Delegate.CreateDelegate(typeof(DelAOTShow), null, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, ref b, 4));
        }

        [UnitTest]
        public void ret_valuetype_aot_close()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));
        }

        [UnitTest]
        public void ret_valuetype_aot_close2()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            DelAOTShow2 del = b.Show;
            Assert.Equal(5, del(4));
        }

        [UnitTest]
        public void ret_valuetype_aot_close_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));
        }

        [UnitTest]
        public void ret_valuetype_intp_open()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Show");
            var del = (ValueTypeShow)Delegate.CreateDelegate(typeof(ValueTypeShow), null, m);
            Assert.Equal(5, del(ref b, 4));
        }

        [UnitTest]
        public void ret_valuetype_intp_close()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Show");
            var del = (Func<int, int>)Delegate.CreateDelegate(typeof(Func<int, int>), b, m);
            Assert.Equal(5, del(4));
        }

        // CallDelegate_ret

        [UnitTest]
        public void expand_ret_class_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Foo");
            var del = (Func<FT_AOT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, del(b, -10));
            Assert.Equal(5, del(b, 4));

            var dd = del + del;
            Assert.Equal(-9, dd(b, -10));
            Assert.Equal(5, dd(b, 4));
        }

        [UnitTest]
        public void expand_ret_class_aot_open_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Foo");
            var del = (Func<FT_AOT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, b, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, b, 4));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, b, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, b, 4));
        }

        [UnitTest]
        public void expand_ret_class_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Foo");
            var del = (Func<FT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, del(b, -10));
            Assert.Equal(5, del(b, 4));

            var dd = del + del;
            Assert.Equal(-9, dd(b, -10));
            Assert.Equal(5, dd(b, 4));
        }

        [UnitTest]
        public void expand_ret_class_aot_close()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(-9, del(-10));
            Assert.Equal(5, del(4));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void expand_ret_class_aot_close_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, 4));
        }

        [UnitTest]
        public void expand_ret_class_intp_close()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(-9, del(-10));
            Assert.Equal(5, del(4));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void expand_ret_class_static_aot_open()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Foo2");
            var del = (Func<FT_AOT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, del(b, -10));
            Assert.Equal(5, del(b, 4));

            var dd = del + del;
            Assert.Equal(-9, dd(b, -10));
            Assert.Equal(5, dd(b, 4));
        }

        [UnitTest]
        public void expand_ret_class_static_aot_open_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = b.GetType().GetMethod("Foo2");
            var del = (Func<FT_AOT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_AOT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, b, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, b, 4));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, b, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, b, 4));
        }

        [UnitTest]
        public void expand_ret_class_static_aot_close()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Foo2");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(-9, del(-10));
            Assert.Equal(5, del(4));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void expand_ret_class_static_aot_close_call_by_aot()
        {
            var b = new FT_AOT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_Class).GetMethod("Foo2");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, 4));
        }

        [UnitTest]
        public void expand_ret_class_static_intp_open()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Foo2");
            var del = (Func<FT_Class, int, sbyte>)Delegate.CreateDelegate(typeof(Func<FT_Class, int, sbyte>), null, m);
            Assert.Equal(-9, del(b, -10));
            Assert.Equal(5, del(b, 4));

            var dd = del + del;
            Assert.Equal(-9, dd(b, -10));
            Assert.Equal(5, dd(b, 4));
        }

        [UnitTest]
        public void expand_ret_class_static_intp_close()
        {
            var b = new FT_Class() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_Class).GetMethod("Foo2");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(5, del(4));
            Assert.Equal(-9, del(-10));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void expand_ret_valuetype_aot_open()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Foo");
            var del = (DelAOTFoo)Delegate.CreateDelegate(typeof(DelAOTFoo), null, m);
            Assert.Equal(5, del(ref b, 4));
            Assert.Equal(-9, del(ref b, -10));

            var dd = del + del;
            Assert.Equal(-9, dd(ref b, -10));
            Assert.Equal(5, dd(ref b, 4));
        }

        [UnitTest]
        public void expand_ret_valuetype_aot_open_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Foo");
            var del = (DelAOTFoo)Delegate.CreateDelegate(typeof(DelAOTFoo), null, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, ref b, 4));
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, ref b, -10));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, ref b, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, ref b, 4));
        }

        [UnitTest]
        public void expand_ret_valuetype_aot_close()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(5, del(4));
            Assert.Equal(-9, del(-10));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void expand_ret_valuetype_aot_close_call_by_aot()
        {
            var b = new FT_AOT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_AOT_ValueType).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(5, CallDelegateFuncs.Invoke(del, 4));
            Assert.Equal(-9, CallDelegateFuncs.Invoke(del, -10));

            var dd = del + del;
            Assert.Equal(-9, CallDelegateFuncs.Invoke(dd, -10));
            Assert.Equal(5, CallDelegateFuncs.Invoke(dd, 4));
        }

        [UnitTest]
        public void expand_ret_valuetype_intp_open()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Foo");
            var del = (ValueTypeFoo)Delegate.CreateDelegate(typeof(ValueTypeFoo), null, m);
            Assert.Equal(5, del(ref b, 4));
            Assert.Equal(-9, del(ref b, -10));

            var dd = del + del;
            Assert.Equal(-9, dd(ref b, -10));
            Assert.Equal(5, dd(ref b, 4));
        }

        [UnitTest]
        public void expand_ret_valuetype_intp_close()
        {
            var b = new FT_ValueType() { x = 1, y = 2f, z = "abc" };
            var m = typeof(FT_ValueType).GetMethod("Foo");
            var del = (Func<int, sbyte>)Delegate.CreateDelegate(typeof(Func<int, sbyte>), b, m);
            Assert.Equal(5, del(4));
            Assert.Equal(-9, del(-10));

            var dd = del + del;
            Assert.Equal(-9, dd(-10));
            Assert.Equal(5, dd(4));
        }

        [UnitTest]
        public void struct_instance_method_call_by_aot()
        {
            var s = new FT_ValueType { x = 1 };
            Func<int> f = s.Foo2;
            int r = CallDelegateFuncs.Invoke(f);
            Assert.Equal(1, r);
        }

        [UnitTest]
        public void struct_instance_method_call_by_aot2()
        {
            var s = new FT_ValueType { x = 1 };
            var m = typeof(FT_ValueType).GetMethod("Foo2");
            var f = (Func<int>)Delegate.CreateDelegate(typeof(Func<int>), s, m);
            int r = CallDelegateFuncs.Invoke(f);
            Assert.Equal(1, r);
        }

        [UnitTest]
        public void struct_instance_method_call_by_aot3()
        {
            var s = new FT_ValueType { x = 1 };
            Func<int> f = s.Foo2;
            int r = f();
            Assert.Equal(1, r);
        }
    }
}
