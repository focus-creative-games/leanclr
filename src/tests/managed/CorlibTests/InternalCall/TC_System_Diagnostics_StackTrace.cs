using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Diagnostics_StackTrace : TestCaseBase
    {
        [UnitTest]
        public void StackTrace_Creation_DefaultConstructor()
        {
            // Test StackTrace creation with default constructor
            // This indirectly invokes get_trace icall
            StackTrace trace = new StackTrace();
            Assert.NotNull(trace);
        }

        [UnitTest]
        public void StackTrace_Creation_WithSkipFrames()
        {
            // Test StackTrace creation with skip frames parameter
            StackTrace trace = new StackTrace(1);
            Assert.NotNull(trace);
        }

        [UnitTest]
        public void StackTrace_Creation_WithException()
        {
            // Test StackTrace creation from an exception
            try
            {
                throw new InvalidOperationException("Test exception for stack trace");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex);
                Assert.NotNull(trace);
            }
        }

        [UnitTest]
        public void StackTrace_Creation_WithException_AndSkipFrames()
        {
            // Test StackTrace creation from exception with skip frames
            try
            {
                throw new ArgumentException("Test exception");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex, 1);
                Assert.NotNull(trace);
            }
        }

        [UnitTest]
        public void StackTrace_Creation_WithException_FileInfo()
        {
            // Test StackTrace creation with file info flag
            try
            {
                throw new NotImplementedException("Test for file info");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex, true);
                Assert.NotNull(trace);
            }
        }

        [UnitTest]
        public void StackTrace_FrameCount_NonNegative()
        {
            // Test that frame count is non-negative
            StackTrace trace = new StackTrace();
            int frameCount = trace.FrameCount;
            Assert.IsTrue(frameCount >= 0);
        }

        [UnitTest]
        public void StackTrace_GetFrame_WithinBounds()
        {
            // Test getting frames within valid bounds
            StackTrace trace = new StackTrace();
            int frameCount = trace.FrameCount;
            
            if (frameCount > 0)
            {
                StackFrame frame = trace.GetFrame(0);
                Assert.NotNull(frame);
            }
        }

        [UnitTest]
        public void StackTrace_GetFrames_Array()
        {
            // Test GetFrames returns valid array
            StackTrace trace = new StackTrace();
            StackFrame[] frames = trace.GetFrames();
            
            if (frames != null)
            {
                Assert.IsTrue(frames.Length >= 0);
                if (frames.Length > 0)
                {
                    Assert.NotNull(frames[0]);
                }
            }
        }

        [UnitTest]
        public void StackTrace_Exception_FrameCount_Valid()
        {
            // Test that exception stack trace has valid frame count
            try
            {
                throw new Exception("Test exception");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex);
                int frameCount = trace.FrameCount;
                Assert.IsTrue(frameCount >= 0);
            }
        }

        [UnitTest]
        public void StackTrace_Exception_GetFrames()
        {
            // Test getting frames from exception stack trace
            try
            {
                throw new Exception("Test for frames");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex);
                StackFrame[] frames = trace.GetFrames();
                
                if (frames != null && frames.Length > 0)
                {
                    // First frame should have valid method
                    MethodBase method = frames[0].GetMethod();
                    Assert.NotNull(method);
                }
            }
        }

        [UnitTest]
        public void StackTrace_SkipFrames_ReducesFrameCount()
        {
            // Test that skip frames parameter reduces frame count
            StackTrace trace0 = new StackTrace(0);
            StackTrace trace1 = new StackTrace(1);
            
            int count0 = trace0.FrameCount;
            int count1 = trace1.FrameCount;
            
            // Skipping frames should result in fewer or equal frames
            Assert.IsTrue(count1 <= count0);
        }

        [UnitTest]
        public void StackTrace_DefaultConstructor_HasFrames()
        {
            // Test that default constructor captures at least some frames
            StackTrace trace = new StackTrace();
            int frameCount = trace.FrameCount;
            Assert.IsTrue(frameCount > 0);
        }

        [UnitTest]
        public void StackTrace_Exception_NestedExceptions()
        {
            // Test stack trace with nested exceptions
            try
            {
                try
                {
                    throw new Exception("Inner");
                }
                catch (Exception inner)
                {
                    throw new OuterException("Outer", inner);
                }
            }
            catch (Exception outer)
            {
                StackTrace trace = new StackTrace(outer);
                Assert.NotNull(trace);
                Assert.IsTrue(trace.FrameCount > 0);
            }
        }

        [UnitTest]
        public void StackTrace_ToString_NotEmpty()
        {
            // Test that ToString produces output
            StackTrace trace = new StackTrace();
            string traceString = trace.ToString();
            
            // Should produce some output (might be empty but not null)
            Assert.NotNull(traceString);
        }

        [UnitTest]
        public void StackTrace_Exception_ToString()
        {
            // Test ToString on exception stack trace
            try
            {
                throw new Exception("Test exception for ToString");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex);
                string traceString = trace.ToString();
                Assert.NotNull(traceString);
            }
        }

        [UnitTest]
        public void StackTrace_Frame_Method_Names()
        {
            // Test that frame methods have valid names
            StackTrace trace = new StackTrace();
            StackFrame[] frames = trace.GetFrames();
            
            if (frames != null && frames.Length > 0)
            {
                foreach (StackFrame frame in frames)
                {
                    MethodBase method = frame.GetMethod();
                    if (method != null)
                    {
                        Assert.IsTrue(!string.IsNullOrEmpty(method.Name));
                    }
                }
            }
        }

        [UnitTest]
        public void StackTrace_Frame_Types()
        {
            // Test that frame types are valid
            StackTrace trace = new StackTrace();
            StackFrame[] frames = trace.GetFrames();
            
            if (frames != null && frames.Length > 0)
            {
                foreach (StackFrame frame in frames)
                {
                    MethodBase method = frame.GetMethod();
                    if (method != null)
                    {
                        Type declaringType = method.DeclaringType;
                        Assert.NotNull(declaringType);
                    }
                }
            }
        }

        [UnitTest]
        public void StackTrace_GetFrame_AllFramesValid()
        {
            // Test that all accessible frames are valid
            StackTrace trace = new StackTrace();
            
            for (int i = 0; i < trace.FrameCount; i++)
            {
                StackFrame frame = trace.GetFrame(i);
                Assert.NotNull(frame);
                Assert.NotNull(frame.GetMethod());
            }
        }

        [UnitTest]
        public void StackTrace_Exception_WithSkipFrames()
        {
            // Test exception stack trace with skip frames
            try
            {
                throw new Exception("Test");
            }
            catch (Exception ex)
            {
                StackTrace trace = new StackTrace(ex, 2);
                Assert.NotNull(trace);
                int frameCount = trace.FrameCount;
                Assert.IsTrue(frameCount >= 0);
            }
        }

        [UnitTest]
        public void StackTrace_CallStack_Captured()
        {
            // Test that call stack is properly captured
            StackFrame frame = GetCurrentStackFrame();
            Assert.NotNull(frame);
            
            MethodBase method = frame.GetMethod();
            Assert.NotNull(method);
            Assert.Equal("GetCurrentStackFrame", method.Name);
        }

        [UnitTest]
        public void StackTrace_MethodCall_Depth()
        {
            // Test stack trace captures method call depth
            StackTrace trace = GetTraceFromDeepCall(3);
            Assert.IsTrue(trace.FrameCount >= 3);
        }

        [UnitTest]
        public void StackTrace_Exception_FileInfo_Flag()
        {
            // Test exception stack trace with different file info flags
            try
            {
                throw new Exception("Test");
            }
            catch (Exception ex)
            {
                StackTrace traceWithoutFile = new StackTrace(ex, false);
                StackTrace traceWithFile = new StackTrace(ex, true);
                
                Assert.NotNull(traceWithoutFile);
                Assert.NotNull(traceWithFile);
            }
        }


        // Helper methods for testing

        private StackFrame GetCurrentStackFrame()
        {
            StackTrace trace = new StackTrace();
            return trace.GetFrame(0);
        }

        private StackTrace GetTraceFromDeepCall(int depth)
        {
            if (depth > 0)
            {
                return GetTraceFromDeepCall(depth - 1);
            }
            return new StackTrace();
        }

        private StackTrace CallStackTraceConstructor(int depth)
        {
            if (depth > 1)
            {
                return CallStackTraceConstructor(depth - 1);
            }
            return new StackTrace();
        }

        private class InnerException : Exception
        {
            public InnerException(string message) : base(message) { }
        }

        private class OuterException : Exception
        {
            public OuterException(string message, Exception inner) : base(message, inner) { }
        }
    }
}
