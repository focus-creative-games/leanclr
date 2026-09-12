namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibConsole()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_Console));
        }

        public static void RunCorlibConsoleTerminal()
        {
            RunCorlibConsoleKeyAvailableOnly();
            RunCorlibConsoleTreatControlCAsInputOnly();
            RunCorlibConsoleWindowWidthOnly();
        }

        public static void RunCorlibConsoleKeyAvailableOnly()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibConsoleNet10Semantics), "KeyAvailable_RedirectedInputThrowsOrReturnsFalse");
        }

        public static void RunCorlibConsoleTreatControlCAsInputOnly()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibConsoleNet10Semantics), "TreatControlCAsInput_RedirectedInputThrowsOrToggles");
        }

        public static void RunCorlibConsoleWindowWidthOnly()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Console), "WindowWidth_ReadWithoutThrow");
        }
    }

    internal class CorlibConsoleNet10Semantics
    {
        [UnitTest]
        public void KeyAvailable_RedirectedInputThrowsOrReturnsFalse()
        {
            try
            {
                bool available = System.Console.KeyAvailable;
                Assert.False(available);
            }
            catch (System.InvalidOperationException)
            {
                // .NET 10 throws when standard input is redirected or backed by a file.
            }
        }

        [UnitTest]
        public void TreatControlCAsInput_RedirectedInputThrowsOrToggles()
        {
            bool previous;
            try
            {
                previous = System.Console.TreatControlCAsInput;
            }
            catch (System.IO.IOException)
            {
                // .NET 10 throws when the input handle is not an interactive console.
                return;
            }

            try
            {
                System.Console.TreatControlCAsInput = !previous;
                Assert.Equal(!previous, System.Console.TreatControlCAsInput);
            }
            finally
            {
                System.Console.TreatControlCAsInput = previous;
            }
        }
    }
}
