using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Environment : TestCaseBase
    {
        [UnitTest]
        public void GetExitCode_Zero()
        {
            int exitCode = Environment.ExitCode;
            Assert.Equal(0, exitCode);
        }

        [UnitTest]
        public void SetExitCode_NonZero()
        {
            Environment.ExitCode = 42;
            int exitCode = Environment.ExitCode;
            Assert.Equal(42, exitCode);
            // Reset to 0 for other tests
            Environment.ExitCode = 0;
        }

        [UnitTest]
        public void HasShutdownStarted_False()
        {
            bool hasShutdownStarted = Environment.HasShutdownStarted;
            Assert.False(hasShutdownStarted);
        }

        [UnitTest]
        public void MachineName_NotEmpty()
        {
            string machineName = Environment.MachineName;
            Assert.True(machineName.Length > 0);
        }

        [UnitTest]
        public void NewLine_CorrectValue()
        {
            string newLine = Environment.NewLine;
            Assert.True(newLine == "\n" || newLine == "\r\n"); // Accept both LF and CRLF depending on platform
        }

        [UnitTest]
        public void GetPlatform_Unix()
        {
            PlatformID platform = Environment.OSVersion.Platform;
            Assert.True(platform == PlatformID.Unix || platform == PlatformID.Win32NT || platform == PlatformID.MacOSX);
        }

        [UnitTest]
        public void GetOSVersionString_NotEmpty()
        {
            string osVersion = Environment.OSVersion.VersionString;
            Assert.False(string.IsNullOrEmpty(osVersion));
        }

        [UnitTest]
        public void GetUserName_NotEmpty()
        {
            string userName = Environment.UserName;
            Assert.True(userName.Length > 0);
        }


        [UnitTest]
        public void Is64BitOperatingSystem_True()
        {
            bool is64Bit = Environment.Is64BitOperatingSystem;
            if (IntPtr.Size == 8)
            {
                Assert.True(is64Bit);
            }
            else
            {
                Assert.False(is64Bit);
            }
        }

        [UnitTest]
        public void GetProcessorCount_Positive()
        {
            int processorCount = Environment.ProcessorCount;
            Assert.IsTrue(processorCount > 0);
        }

        [UnitTest]
        public void GetPageSize_4096()
        {
            int pageSize = Environment.SystemPageSize;
            Assert.Equal(4096, pageSize);
        }

        [UnitTest]
        public void GetEnvironmentVariable_Null()
        {
            string value = Environment.GetEnvironmentVariable("NON_EXISTENT_ENV_VAR");
            Assert.Null(value);
        }

        [UnitTest]
        public void SetEnvironmentVariable_And_Get()
        {
            string varName = "TEST_ENV_VAR";
            string varValue = "TestValue";
            Environment.SetEnvironmentVariable(varName, varValue);
            string retrievedValue = Environment.GetEnvironmentVariable(varName);
            Assert.Equal(varValue, retrievedValue);
        }
    }
}
