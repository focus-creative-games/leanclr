using System;
using System.IO;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Console : TestCaseBase
    {
        [UnitTest]
        [CoversIcall("System.ConsoleDriver::Isatty(System.IntPtr)")]
        [CoversIcall("System.ConsoleDriver::TtySetup(System.String,System.String,System.Byte[]&,System.Int32*&)")]
        public void WriteLine_String_Redirect()
        {
            var buffer = new StringWriter();
            TextWriter previousOut = Console.Out;
            try
            {
                Console.SetOut(buffer);
                Console.WriteLine("leanclr-console-test");
                Assert.Equal("leanclr-console-test" + Environment.NewLine, buffer.ToString());
            }
            finally
            {
                Console.SetOut(previousOut);
            }
        }

        [UnitTest]
        public void WriteLine_NoArg_WritesNewlineOnly()
        {
            var buffer = new StringWriter();
            TextWriter previousOut = Console.Out;
            try
            {
                Console.SetOut(buffer);
                Console.WriteLine();
                Assert.Equal(Environment.NewLine, buffer.ToString());
            }
            finally
            {
                Console.SetOut(previousOut);
            }
        }

        [UnitTest]
        public void WriteLine_Int32_FormatsValue()
        {
            var buffer = new StringWriter();
            TextWriter previousOut = Console.Out;
            try
            {
                Console.SetOut(buffer);
                Console.WriteLine(42);
                Assert.Equal("42" + Environment.NewLine, buffer.ToString());
            }
            finally
            {
                Console.SetOut(previousOut);
            }
        }

        [UnitTest]
        public void WriteLine_CompositeFormat_FormatsArguments()
        {
            var buffer = new StringWriter();
            TextWriter previousOut = Console.Out;
            try
            {
                Console.SetOut(buffer);
                Console.WriteLine("{0}:{1}", "a", 7);
                Assert.Equal("a:7" + Environment.NewLine, buffer.ToString());
            }
            finally
            {
                Console.SetOut(previousOut);
            }
        }

        [UnitTest]
        public void WriteError_String_Redirect()
        {
            var buffer = new StringWriter();
            TextWriter previousError = Console.Error;
            try
            {
                Console.SetError(buffer);
                Console.Error.WriteLine("leanclr-console-error");
                Assert.Equal("leanclr-console-error" + Environment.NewLine, buffer.ToString());
            }
            finally
            {
                Console.SetError(previousError);
            }
        }

        [UnitTest]
        public void StandardStreams_AreAvailable()
        {
            Assert.NotNull(Console.Out);
            Assert.NotNull(Console.Error);
            Assert.NotNull(Console.In);
        }

        [UnitTest]
        public void WriteLine_StandardOutput_DoesNotThrow()
        {
            Console.WriteLine("leanclr-console-smoke");
        }

        [UnitTest]
        [CoversIcall("System.ConsoleDriver::InternalKeyAvailable(System.Int32)")]
        public void KeyAvailable_ReadWithoutThrow()
        {
            bool available = Console.KeyAvailable;
            Assert.False(available);
        }

        [UnitTest]
        [CoversIcall("System.ConsoleDriver::SetBreak(System.Boolean)")]
        public void TreatControlCAsInput_ToggleWithoutThrow()
        {
            bool previous = Console.TreatControlCAsInput;
            try
            {
                Console.TreatControlCAsInput = true;
                Assert.True(Console.TreatControlCAsInput);
                Console.TreatControlCAsInput = false;
                Assert.False(Console.TreatControlCAsInput);
            }
            finally
            {
                Console.TreatControlCAsInput = previous;
            }
        }

        [UnitTest]
        public void WindowWidth_ReadWithoutThrow()
        {
            try
            {
                int width = Console.WindowWidth;
                Assert.True(width >= 0);
            }
            catch (IOException)
            {
                // No interactive console (e.g. redirected or non-tty host).
            }
        }
    }
}
