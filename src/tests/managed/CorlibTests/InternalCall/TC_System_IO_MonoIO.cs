using System;
using System.IO;

namespace CorlibTests.InternalCall
{
    internal class TC_System_IO_MonoIO : TestCaseBase
    {
        [CoversIcall("System.IO.MonoIO::get_DirectorySeparatorChar")]
        [UnitTest]
        public void DirectorySeparatorChar_IsNonZero()
        {
            char c = Path.DirectorySeparatorChar;
            Assert.IsTrue(c == '\\' || c == '/');
        }

        [CoversIcall("System.IO.MonoIO::get_AltDirectorySeparatorChar")]
        [UnitTest]
        public void AltDirectorySeparatorChar_IsDefined()
        {
            char c = Path.AltDirectorySeparatorChar;
            Assert.IsTrue(c != '\0');
        }

        [CoversIcall("System.IO.MonoIO::get_PathSeparator")]
        [UnitTest]
        public void PathSeparator_IsDefined()
        {
            char c = Path.PathSeparator;
            Assert.IsTrue(c != '\0');
        }

        [CoversIcall("System.IO.MonoIO::get_VolumeSeparatorChar")]
        [UnitTest]
        public void VolumeSeparatorChar_IsDefined()
        {
            char c = Path.VolumeSeparatorChar;
            Assert.IsTrue(c != '\0');
        }

        [CoversIcall("System.IO.MonoIO::GetCurrentDirectory(System.IO.MonoIOError&)")]
        [UnitTest]
        public void GetCurrentDirectory_ReturnsNonEmpty()
        {
            string dir = Directory.GetCurrentDirectory();
            Assert.NotNull(dir);
            Assert.IsTrue(dir.Length > 0);
        }

        [CoversIcall("System.IO.MonoIO::Open(System.Char*,System.IO.FileMode,System.IO.FileAccess,System.IO.FileShare,System.IO.FileOptions,System.IO.MonoIOError&)")]
        [CoversIcall("System.IO.MonoIO::Read(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)")]
        [CoversIcall("System.IO.MonoIO::Write(System.IntPtr,System.Byte[],System.Int32,System.Int32,System.IO.MonoIOError&)")]
        [CoversIcall("System.IO.MonoIO::Seek(System.IntPtr,System.Int64,System.IO.SeekOrigin,System.IO.MonoIOError&)")]
        [CoversIcall("System.IO.MonoIO::GetLength(System.IntPtr,System.IO.MonoIOError&)")]
        [CoversIcall("System.IO.MonoIO::Close(System.IntPtr,System.IO.MonoIOError&)")]
        [UnitTest]
        public void FileStream_ReadWriteRoundTrip()
        {
            string path = Path.Combine(Path.GetTempPath(), "leanclr_monoio_" + Guid.NewGuid().ToString("N") + ".bin");
            byte[] payload = { 1, 2, 3, 4 };
            try
            {
                File.WriteAllBytes(path, payload);
                byte[] read = File.ReadAllBytes(path);
                Assert.Equal(payload.Length, read.Length);
                Assert.Equal(1, read[0]);
                Assert.Equal(4, read[3]);
            }
            finally
            {
                if (File.Exists(path))
                    File.Delete(path);
            }
        }

        [CoversIcall("System.IO.MonoIO::RemapPath(System.String,System.String&)")]
        [UnitTest]
        public void RemapPath_NoChangeForSimplePath()
        {
            string path = "test.txt";
            string full = Path.GetFullPath(path);
            Assert.NotNull(full);
            Assert.IsTrue(full.Length > 0);
        }
    }
}
