using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Diagnostics_StackFrame : TestCaseBase
    {
        [UnitTest]
        public void StackFrame_Creation_NoFileInfo()
        {
            // Test StackFrame creation without file info
            // This indirectly invokes get_frame_info via StackFrame constructor
            StackFrame frame = new StackFrame(0, false);
            Assert.NotNull(frame);
        }

        [UnitTest]
        public void StackFrame_Creation_WithFileInfo()
        {
            // Test StackFrame creation with file info
            StackFrame frame = new StackFrame(0, true);
            Assert.NotNull(frame);
        }

        [UnitTest]
        public void StackFrame_GetMethod()
        {
            // Test that GetMethod works correctly
            // This indirectly invokes get_frame_info to populate method info
            StackFrame frame = new StackFrame(0, false);
            MethodBase method = frame.GetMethod();
            Assert.NotNull(method);
        }

        [UnitTest]
        public void StackFrame_GetFileLineNumber()
        {
            // Test that GetFileLineNumber returns a valid integer
            StackFrame frame = new StackFrame(0, true);
            int lineNumber = frame.GetFileLineNumber();
            // Line number should be >= 0 or -1 if not available
            Assert.IsTrue(lineNumber >= -1);
        }

        [UnitTest]
        public void StackFrame_GetFileColumnNumber()
        {
            // Test that GetFileColumnNumber returns a valid integer
            StackFrame frame = new StackFrame(0, true);
            int columnNumber = frame.GetFileColumnNumber();
            // Column number should be >= 0 or -1 if not available
            Assert.IsTrue(columnNumber >= -1);
        }

        [UnitTest]
        public void StackFrame_GetFileName()
        {
            // Test that GetFileName returns a string or null
            StackFrame frame = new StackFrame(0, true);
            string fileName = frame.GetFileName();
            // Should be null or a valid path string
        }

        [UnitTest]
        public void StackFrame_GetILOffset()
        {
            // Test that GetILOffset returns a valid integer
            StackFrame frame = new StackFrame(0, false);
            int ilOffset = frame.GetILOffset();
            // IL offset should be >= -1
            Assert.IsTrue(ilOffset >= -1);
        }

        [UnitTest]
        public void StackFrame_GetNativeOffset()
        {
            // Test that GetNativeOffset returns a valid integer
            StackFrame frame = new StackFrame(0, false);
            int nativeOffset = frame.GetNativeOffset();
            // Native offset should be >= -1
            Assert.IsTrue(nativeOffset >= -1);
        }

        [UnitTest]
        public void StackFrame_MethodInfo_Consistency()
        {
            // Test that method info is consistent across calls
            StackFrame frame = new StackFrame(0, false);
            MethodBase method1 = frame.GetMethod();
            MethodBase method2 = frame.GetMethod();
            
            if (method1 != null && method2 != null)
            {
                Assert.Equal(method1.Name, method2.Name);
            }
        }

        [UnitTest]
        public void StackFrame_MultipleFrames()
        {
            // Test creating multiple stack frames
            StackFrame frame0 = new StackFrame(0, false);
            StackFrame frame1 = new StackFrame(1, false);
            StackFrame frame2 = new StackFrame(2, false);
            
            Assert.NotNull(frame0);
            Assert.NotNull(frame1);
            Assert.NotNull(frame2);
        }

        [UnitTest]
        public void StackFrame_DifferentFramesHaveDifferentMethods()
        {
            // Test that different frames may have different methods
            StackFrame frame0 = new StackFrame(0, false);
            StackFrame frame1 = new StackFrame(1, false);
            
            MethodBase method0 = frame0.GetMethod();
            MethodBase method1 = frame1.GetMethod();
            
            // At least one should have a valid method
            Assert.IsTrue(method0 != null || method1 != null);
        }

        [UnitTest]
        public void StackFrame_FileInfo_WithNullFileName()
        {
            // Test StackFrame with file info when file name is not available
            StackFrame frame = new StackFrame(0, true);
            string fileName = frame.GetFileName();
            int lineNumber = frame.GetFileLineNumber();
            
            // If file name is null, line number should be 0 or -1
            if (fileName == null)
            {
                Assert.IsTrue(lineNumber == 0 || lineNumber == -1);
            }
        }

        [UnitTest]
        public void StackFrame_NoFileInfo_vs_WithFileInfo()
        {
            // Test difference between with and without file info
            StackFrame frameNoFile = new StackFrame(0, false);
            StackFrame frameWithFile = new StackFrame(0, true);
            
            // Both should have valid method
            Assert.NotNull(frameNoFile.GetMethod());
            Assert.NotNull(frameWithFile.GetMethod());
        }

        [UnitTest]
        public void StackFrame_Method_Name_NotEmpty()
        {
            // Test that method name is not empty when available
            StackFrame frame = new StackFrame(0, false);
            MethodBase method = frame.GetMethod();
            
            if (method != null)
            {
                Assert.IsTrue(!string.IsNullOrEmpty(method.Name));
            }
        }

        [UnitTest]
        public void StackFrame_DeclaringType_Valid()
        {
            // Test that declaring type is valid when available
            StackFrame frame = new StackFrame(0, false);
            MethodBase method = frame.GetMethod();
            
            if (method != null)
            {
                Type declaringType = method.DeclaringType;
                Assert.NotNull(declaringType);
            }
        }

        [UnitTest]
        public void StackFrame_OffsetValues_Valid()
        {
            // Test that offset values are valid
            StackFrame frame = new StackFrame(0, false);
            int ilOffset = frame.GetILOffset();
            int nativeOffset = frame.GetNativeOffset();
            
            // Both should be >= -1
            Assert.IsTrue(ilOffset >= -1);
            Assert.IsTrue(nativeOffset >= -1);
        }

        [UnitTest]
        public void StackFrame_ColumnNumber_Valid()
        {
            // Test that column number is valid or -1
            StackFrame frame = new StackFrame(0, true);
            int columnNumber = frame.GetFileColumnNumber();
            
            // Column number should be -1 or >= 0
            Assert.IsTrue(columnNumber == -1 || columnNumber >= 0);
        }

        [UnitTest]
        public void StackFrame_LineNumber_Valid()
        {
            // Test that line number is valid or -1
            StackFrame frame = new StackFrame(0, true);
            int lineNumber = frame.GetFileLineNumber();
            
            // Line number should be -1 or >= 0
            Assert.IsTrue(lineNumber == -1 || lineNumber >= 0);
        }

        [UnitTest]
        public void StackFrame_Constructor_WithDifferentSkips()
        {
            // Test constructors with different skip values
            try
            {
                StackFrame frame0 = new StackFrame(0, false);
                StackFrame frame1 = new StackFrame(1, false);
                StackFrame frame5 = new StackFrame(5, false);
                
                Assert.NotNull(frame0);
                Assert.NotNull(frame1);
                Assert.NotNull(frame5);
            }
            catch (Exception ex)
            {
                Assert.Fail($"StackFrame constructor threw unexpected exception: {ex.Message}");
            }
        }
    }
}
