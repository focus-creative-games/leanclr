using System;

   class TC_conv_ovf_u8
   {
       [UnitTest]
       public void float_1()
       {
           float x = 1;
           checked
           {
               ulong y = (ulong)x;
               Assert.Equal(1, y);
           }
       }

       [UnitTest]
       public void float_overflow_up()
       {
           float x = 3.40282e+020f;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void float_overflow_down()
       {
           float x = -1;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void double_1()
       {
           double x = 1;
           checked
           {
               ulong y = (ulong)x;
               Assert.Equal(1, y);
           }
       }

       [UnitTest]
       public void double_overflow_up()
       {
           double x = 3.40282e+020f;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void double_overflow_down()
       {
           double x = -1;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void float_Infinity()
       {
           float x = float.PositiveInfinity;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void float_NegativeInfinity()
       {
           float x = float.NegativeInfinity;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void double_Infinity()
       {
           double x = double.PositiveInfinity;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }

       [UnitTest]
       public void double_NegativeInfinity()
       {
           double x = double.NegativeInfinity;
           Assert.ExpectException<OverflowException>(() =>
           {
               checked
               {
                   ulong y = (ulong)x;
               }
           });
       }
   }
