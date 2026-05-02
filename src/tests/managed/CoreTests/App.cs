using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Dynamic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using test;

namespace CoreTests
{

    public class A
    {
        public int a;
        public int b;
        public int c;
    }

    public static class B
    {
        public static int a;
        public static int b;
        public static int c;
    }

    public abstract class C
    {
        public virtual int GetValue()
        {
            return 1;
        }
    }

    public class D : C
    {
        public override int GetValue()
        {
            return 2;
        }
    }

    public partial class App
    {
        public static void Main()
        {

        }

        public static void CallCustomPInvoke()
        {
            Debugger.Log(0, "", $"3+4={CustomPInvoke.Add(3, 4)}");
        }

        static void Throw1()
        {
            throw new Exception();
        }

        static void PrintStackTrace()
        {
            Debugger.Log(0, "", Environment.StackTrace);
        }

        public static int Run(int a, int b)
        {
            return a + b;
        }

        public static string Test1()
        {
            return "Hello, World!";
        }

        public static int Test2()
        {
            int a = 0;
            goto xx;
            return 1;
        yy: a = 2;
            return 2;
        xx:
            a = 1;
            goto yy;

            return 3;
        }

        public static int Test3()
        {
            bool a = false;
            if (a)
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }

        public static int Test4(int a, int b)
        {
            if (a < b)
            {
                return 1;
            }
            return 2;
        }

        public static int Test5(int a)
        {
            switch (a)
            {
                case 1:
                    return 1;
                case 2:
                    return 2;
                default:
                    return 3;
            }
        }

        public static int Test6(int a)
        {
            ref int b = ref a;
            return b;
        }

        public static int Test7(int a)
        {
            ref int b = ref a;
            b = 10;
            return b;
        }

        public static int Test8(int a)
        {
            byte b = (byte)a;
            return b;
        }

        public static int Test9(int a, int b)
        {
            bool c = a < b;
            return c ? 1 : -2;
        }

        public static int Test10(object a)
        {
            return a is int ? 1 : 2;
        }

        public static int Test11(object a)
        {
            return a is string ? 1 : 2;
        }

        public static object Test12(int a)
        {
            return a;
        }

        public static int Test13(object a)
        {
            return (int)a;
        }

        public static byte[] Test20(int len)
        {
            return new byte[len];
        }

        public static int Test21(int[] arr)
        {
            return arr.Length;
        }

        public static void Test22(int[] arr)
        {
            ref int b = ref arr[0];
            b = 10;
        }

        public static int Test23(int[] arr)
        {
            return arr[0];
        }

        public static void Test24(int[] arr, int value)
        {
            arr[0] = value;
        }

        public static int Test30()
        {
            int a = 10;
            var r = __makeref(a);
            var t = __reftype(r);
            var v = __refvalue(r, int);
            return a;
        }

        public unsafe static int Test40()
        {
            var a = stackalloc int[3];
            a[0] = 10;
            return a[0];
        }

        public static int Test50(A a)
        {
            return a.b;
        }

        public static void Test51(A a, int x)
        {
            a.b = x;
        }

        public static void Test52(A a, int x)
        {
            ref int fa = ref a.b;
            fa = x;
        }

        public static int Test53()
        {
            return B.b;
        }

        public static void Test54(int x)
        {
            B.b = x;
        }

        public static void Test55(int x)
        {
            ref int fb = ref B.b;
            fb = x;
        }

        private static int GetA(int x)
        {
            return x;
        }

        public static int Test60(int x)
        {
            return GetA(x);
        }

        public static int Test61()
        {
            C c = new D();
            return c.GetValue();
        }

        public static int Test70()
        {
            int a = 0;
            try
            {
                a = 11;
            }
            catch (Exception)
            {
                a = 22;
            }
            return a;
        }


        //public int Test71()
        //{
        //    return Test70();
        //}

        public static int Test71()
        {
            int a = 0;

            try
            {
                a += 1;
            }
            finally
            {
                try
                {
                    a += 10;
                }
                finally
                {
                    a += 100;
                }
                a += 1000;
            }

            return a;
        }


        //public int Test73()
        //{
        //    return Test72();
        //}

        public static int Test72()
        {
            int a = 0;

            try
            {
                a += 1;
            }
            finally
            {
                try
                {
                    a += 10;
                    throw new Exception();
                }
                catch (Exception e)
                {
                    a += 20;
                }
                finally
                {
                    a += 100;
                }
                a += 1000;
            }

            return a;
        }


        //public void finally_catch()
        //{
        //    Assert.Equal(1131, FinallyCatchException_Return1131());
        //}

        public static int Test73()
        {
            int a = 0;
            try
            {
                try
                {
                    a += 1;
                }
                finally
                {
                    try
                    {
                        a += 10;
                        throw new Exception();
                    }
                    finally
                    {
                        a += 100;
                    }

                    a += 1000;
                }
            }
            catch (Exception)
            {
                a += 10000;
            }

            return a;
        }


        //public void finally_notcatch()
        //{
        //    Assert.Equal(10111, FinallyThrowNotCatch_Return10111());
        //}

        public static int Test74()
        {
            int a = 0;

            try
            {
                a += 1;
                throw new Exception();
            }
            catch (Exception)
            {
                try
                {
                    a += 10;
                }
                finally
                {
                    a += 100;
                }
            }
            finally
            {
                a += 1000;
            }

            return a;
        }



        //public void catch_finally()
        //{
        //    Assert.Equal(1111, CatchFinally_Return1111());
        //}

        public static int Test75()
        {
            int a = 0;

            try
            {
                a += 1;
                throw new Exception();
            }
            catch (Exception)
            {
                try
                {
                    a += 10;
                    throw new Exception();
                }
                catch (Exception)
                {
                    a += 100;
                }
                finally
                {
                    a += 1000;
                }
            }
            finally
            {
                a += 10000;
            }

            return a;
        }



        //public void catch_throw_catch()
        //{
        //    Assert.Equal(11111, CatchThrowCatch_Return11111());
        //}

        public static int Test76()
        {
            int a = 0;
            try
            {
                try
                {
                    a += 1;
                    throw new Exception();
                }
                finally
                {
                    Test70();
                    a += 10;
                }
            }
            catch (Exception)
            {
                a += 100;
            }
            finally
            {
                a += 1000;
            }

            return a;
        }



        //public void throw_finally_call()
        //{
        //    Assert.Equal(1111, ThrowFinallyCallClauseCatchFinally_Return1111());
        //}

        public static int Test77()
        {
            int a = 0;
            try
            {
                try
                {
                    a = 1;
                    throw new Exception();
                }
                finally
                {
                    try
                    {
                        try
                        {
                            a = 3;
                        }
                        catch (Exception)
                        {

                        }
                        a = 4;
                    }
                    catch (Exception)
                    {

                    }
                    a = 2;
                }
            }
            catch (Exception)
            {

            }

            return a;
        }



        //public void ex_2()
        //{
        //    Assert.Equal(2, ThrowExceptionAtFinallySubStatementIncludeLeave_Return2());
        //}

        public static int Test78()
        {
            int a = 0;
            try
            {
                try
                {
                    a = 1;
                    throw new Exception();
                }
                finally
                {
                    try
                    {
                        try
                        {
                            a = 3;
                        }
                        catch (Exception)
                        {

                        }
                        a = 4;
                    }
                    catch (Exception)
                    {

                    }
                    a = 2;
                }
            }
            catch (Exception)
            {

            }

            return a;
        }


        //public void ex_3()
        //{
        //    Assert.Equal(2, LeaveFinallySubLeave_Return2());
        //}

        public static int Test79()
        {
            int a = 0;
            try
            {
                try
                {
                    a = 1;
                    throw new Exception();
                }
                catch (Exception e)
                {
                    try
                    {
                        try
                        {
                            a = 3;
                            throw;
                        }
                        catch (InvalidCastException e2) when (a.GetHashCode() == 7)
                        {
                            throw;
                        }
                        catch (Exception)
                        {

                        }

                        throw;
                        a = 4;
                    }
                    catch (Exception)
                    {

                    }


                    throw;
                    a = 2;
                }
            }
            catch (Exception)
            {

            }

            return a;
        }


        //public void ex_4()
        //{
        //    Assert.Equal(3, Rethrow_Return3());
        //}
    }
}
