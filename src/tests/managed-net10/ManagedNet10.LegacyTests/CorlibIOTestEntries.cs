namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibIO()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(CorlibTests.InternalCall.TC_System_IO_MonoIO));
        }

        public static void RunCorlibIOFileStream()
        {
            new CorlibTests.InternalCall.TC_System_IO_MonoIO().FileStream_ReadWriteRoundTrip();
        }

        public static void RunCorlibIOGuidOnly()
        {
            string text = System.Guid.NewGuid().ToString("N");
            if (text.Length != 32)
            {
                throw new System.Exception("Guid N format length mismatch.");
            }
        }

        public static void RunCorlibIOPathOnly()
        {
            string path = BuildFileStreamPath();
            if (path.Length == 0)
            {
                throw new System.Exception("FileStream path was empty.");
            }
        }

        public static void RunCorlibIOFullPathOnly()
        {
            string path = System.IO.Path.GetFullPath(BuildFileStreamPath());
            if (path.Length == 0)
            {
                throw new System.Exception("FileStream full path was empty.");
            }
        }

        public static void RunCorlibIOWriteOnly()
        {
            string path = BuildFileStreamPath();
            try
            {
                System.IO.File.WriteAllBytes(path, new byte[] { 1, 2, 3, 4 });
            }
            finally
            {
                if (System.IO.File.Exists(path))
                    System.IO.File.Delete(path);
            }
        }

        public static void RunCorlibIOFileExistsOnly()
        {
            string path = BuildFileStreamPath();
            if (System.IO.File.Exists(path))
            {
                throw new System.Exception("New temporary FileStream path unexpectedly existed.");
            }
        }

        public static void RunCorlibIOOpenHandleOnly()
        {
            string path = BuildFileStreamPath();
            Microsoft.Win32.SafeHandles.SafeFileHandle handle = System.IO.File.OpenHandle(
                path,
                System.IO.FileMode.Create,
                System.IO.FileAccess.Write,
                System.IO.FileShare.None);
            if (handle.IsInvalid)
            {
                throw new System.Exception("File.OpenHandle returned an invalid handle.");
            }

            handle.Dispose();
            if (System.IO.File.Exists(path))
            {
                System.IO.File.Delete(path);
            }
        }

        public static void RunCorlibIONewSafeFileHandleOnly()
        {
            using (Microsoft.Win32.SafeHandles.SafeFileHandle handle = new Microsoft.Win32.SafeHandles.SafeFileHandle())
            {
                if (!handle.IsInvalid)
                {
                    throw new System.Exception("Default SafeFileHandle should be invalid.");
                }
            }
        }

        public static void RunCorlibIOActivatorSafeFileHandleOnly()
        {
            using (Microsoft.Win32.SafeHandles.SafeFileHandle handle = System.Activator.CreateInstance<Microsoft.Win32.SafeHandles.SafeFileHandle>())
            {
                if (!handle.IsInvalid)
                {
                    throw new System.Exception("Activator-created SafeFileHandle should be invalid.");
                }
            }
        }

        public static void RunCorlibIOCreateStreamOnly()
        {
            string path = BuildFileStreamPath();
            try
            {
                using (new System.IO.FileStream(path, System.IO.FileMode.Create, System.IO.FileAccess.Write, System.IO.FileShare.None))
                {
                }
            }
            finally
            {
                if (System.IO.File.Exists(path))
                    System.IO.File.Delete(path);
            }
        }

        public static void RunCorlibIOCreateStreamWriteOnly()
        {
            string path = BuildFileStreamPath();
            try
            {
                using (System.IO.FileStream stream = new System.IO.FileStream(path, System.IO.FileMode.Create, System.IO.FileAccess.Write, System.IO.FileShare.None))
                {
                    stream.Write(new byte[] { 1, 2, 3, 4 }, 0, 4);
                }
            }
            finally
            {
                if (System.IO.File.Exists(path))
                    System.IO.File.Delete(path);
            }
        }

        private static string BuildFileStreamPath()
        {
            return System.IO.Path.Combine(System.IO.Path.GetTempPath(), "leanclr_monoio_" + System.Guid.NewGuid().ToString("N") + ".bin");
        }
    }
}
