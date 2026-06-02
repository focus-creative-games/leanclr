using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Exceptions
{
    internal class TC_throw : GeneralTestCaseBase
    {
        public static int SubFuncWithLeave()
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

        [UnitTest]
        public void call_sub_fun()
        {
            Assert.Equal(11, SubFuncWithLeave());
        }

        public static int FinallyFinally_Return1111()
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

        [UnitTest]
        public void finally_finally()
        {
            Assert.Equal(1111, FinallyFinally_Return1111());
        }

        public static int FinallyCatchException_Return1131()
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

        [UnitTest]
        public void finally_catch()
        {
            Assert.Equal(1131, FinallyCatchException_Return1131());
        }

        public static int FinallyThrowNotCatch_Return10111()
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

        [UnitTest]
        public void finally_notcatch()
        {
            Assert.Equal(10111, FinallyThrowNotCatch_Return10111());
        }

        public static int CatchFinally_Return1111()
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


        [UnitTest]
        public void catch_finally()
        {
            Assert.Equal(1111, CatchFinally_Return1111());
        }

        public static int CatchThrowCatch_Return11111()
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


        [UnitTest]
        public void catch_throw_catch()
        {
            Assert.Equal(11111, CatchThrowCatch_Return11111());
        }

        public static int ThrowFinallyCallClauseCatchFinally_Return1111()
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
                    SubFuncWithLeave();
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


        [UnitTest]
        public void throw_finally_call()
        {
            Assert.Equal(1111, ThrowFinallyCallClauseCatchFinally_Return1111());
        }

        public static int ThrowExceptionAtFinallySubStatementIncludeLeave_Return2()
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


        [UnitTest]
        public void ex_2()
        {
            Assert.Equal(2, ThrowExceptionAtFinallySubStatementIncludeLeave_Return2());
        }

        public static int LeaveFinallySubLeave_Return2()
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

        [UnitTest]
        public void ex_3()
        {
            Assert.Equal(2, LeaveFinallySubLeave_Return2());
        }

        public static int Rethrow_Return3()
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

        [UnitTest]
        public void ex_4()
        {
            Assert.Equal(3, Rethrow_Return3());
        }
    }
}
