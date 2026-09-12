using System;

namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunArrayCopyRegressions()
        {
            RunIndexedArrayCopyRegression();
            RunWholeArrayCopyRegression();
        }

        public static void RunCorlibStackTrace()
        {
            LegacyTestRunner.RunMethod(
                typeof(CorlibTests.InternalCall.TC_System_Diagnostics_StackTrace),
                "StackTrace_GetFrames_Array");
        }

        private static void RunIndexedArrayCopyRegression()
        {
            object[] source = new object[] { "first", "second", "third" };
            object[] destination = new object[2];

            Array.Copy(source, 1, destination, 0, destination.Length);

            Assert.Equal("second", (string)destination[0]);
            Assert.Equal("third", (string)destination[1]);
        }

        private static void RunWholeArrayCopyRegression()
        {
            int[] source = new int[] { 7, 11, 13 };
            int[] destination = new int[source.Length];

            Array.Copy(source, destination, source.Length);

            Assert.Equal(7, destination[0]);
            Assert.Equal(11, destination[1]);
            Assert.Equal(13, destination[2]);
        }
    }
}
