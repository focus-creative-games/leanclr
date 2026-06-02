using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class BugException : GeneralTestCaseBase
    {
        [UnitTest]
        public void finally_try_throw_catch()
        {
            int a = 5;
            int b = 4;
            try
            {
                a += b;
            }
            finally
            {
                try
                {
                    throw new ArgumentException();
                }
                catch (Exception e)
                {
                    e.ToString();
                }
            }
        }


        [UnitTest]
        public void finally_try_catch()
        {
            int a = 5;
            int b = 4;
            try
            {
                a += b;
            }
            finally
            {
                try
                {
                    b += a;
                }
                catch (Exception e)
                {
                    e.ToString();
                }
            }
        }
    }
}
