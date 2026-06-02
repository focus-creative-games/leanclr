using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Exceptions
{
    internal class TC_leave : GeneralTestCaseBase
    {

        [UnitTest]
        public void loop_try_catch()
        {
            int k = 0;
            for(int i = 0; i < 1000; i++)
            {
                try
                {
                    k += i;
                }
                catch(Exception)
                {

                }
            }
        }

        //[UnitTest]
        //public void foreach_arr()
        //{
        //    var arr = new int[] { 1, 2 };
        //    int sum = 0;
            
        //    foreach(var e in arr)
        //    {
        //        sum += e;
        //    }
        //    Assert.Equal(3, sum);
        //}


        [UnitTest]
        public void foreach_try_catch()
        {
            int n = 10;
            int k = 0;
            for(int i = 0; i < n;i++)
            {
                try
                {
                    k += i;
                }
                catch (Exception)
                {

                }
            }
        }

        [UnitTest]
        public void foreach_try_catch_exception()
        {
            int n = 10;
            int k = 0;
            for (int i = 0; i < n; i++)
            {
                try
                {
                    throw new Exception();
                }
                catch (Exception)
                {

                }
            }
        }
    }
}
