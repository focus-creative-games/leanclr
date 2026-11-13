using System;
using System.IO;
using System.Reflection;
using System.Runtime.Loader;
namespace AssemblyStreamTests
{
public class ForwardOnly : Stream
{
    byte[] data;
    int offset;
    public ForwardOnly(byte[] d)
    {
        data = d;
    }
    public override bool CanRead
    {
        get {
            return true;
        }
    }
    public override bool CanSeek
    {
        get {
            return false;
        }
    }
    public override bool CanWrite
    {
        get {
            return false;
        }
    }
    public override long Length
    {
        get {
            throw new NotSupportedException();
        }
    }
    public override long Position
    {
        get {
            throw new NotSupportedException();
        }
        set {
            throw new NotSupportedException();
        }
    }
    public override int Read(byte[] b, int o, int n)
    {
        int count = Math.Min(n, Math.Min(7, data.Length - offset));
        Array.Copy(data, offset, b, o, count);
        offset += count;
        return count;
    }
    public override void Flush()
    {
    }
    public override long Seek(long x, SeekOrigin s)
    {
        throw new NotSupportedException();
    }
    public override void SetLength(long x)
    {
        throw new NotSupportedException();
    }
    public override void Write(byte[] b, int o, int n)
    {
        throw new NotSupportedException();
    }
}
public class Chunked : Stream
{
    protected byte[] data;
    protected int offset;
    public Chunked(byte[] d)
    {
        data = d;
    }
    public override bool CanRead
    {
        get {
            return true;
        }
    }
    public override bool CanSeek
    {
        get {
            return false;
        }
    }
    public override bool CanWrite
    {
        get {
            return false;
        }
    }
    public override long Length
    {
        get {
            return data.Length;
        }
    }
    public override long Position
    {
        get {
            return offset;
        }
        set {
            throw new NotSupportedException();
        }
    }
    public override int Read(byte[] b, int o, int n)
    {
        int c = Math.Min(n, Math.Min(7, data.Length - offset));
        Array.Copy(data, offset, b, o, c);
        offset += c;
        return c;
    }
    public override void Flush()
    {
    }
    public override long Seek(long x, SeekOrigin s)
    {
        throw new NotSupportedException();
    }
    public override void SetLength(long x)
    {
        throw new NotSupportedException();
    }
    public override void Write(byte[] b, int o, int n)
    {
        throw new NotSupportedException();
    }
}
public class Broken : Chunked
{
    public Broken(byte[] d) : base(d)
    {
    }
    public override int Read(byte[] b, int o, int n)
    {
        throw new IOException("read failed");
    }
}
public static class Program
{
    static void Check(bool c, string m)
    {
        if (!c)
            throw new Exception(m);
    }
    static Assembly Load(byte[] b)
    {
        using (var s = new MemoryStream(b)) return AssemblyLoadContext.Default.LoadFromStream(s);
    }
    static void Verify(Assembly a)
    {
        Check(a.GetType("StreamFixture.Entry") != null, "type");
        Check(AssemblyLoadContext.GetLoadContext(a) == AssemblyLoadContext.Default, "context identity");
        Check((int)a.GetType("StreamFixture.Entry").GetMethod("Answer").Invoke(null, null) == 42, "method");
    }
    public static void DefaultOnly()
    {
        Check(AssemblyLoadContext.Default != null, "default");
    }
    public static void Memory()
    {
        Load(Data.Dependency());
        Verify(Load(Data.Fixture()));
    }
    public static void Exposed()
    {
        Load(Data.Dependency());
        var b = Data.Fixture();
        using (var s = new MemoryStream(b, 0, b.Length, false, true))
        {
            Verify(AssemblyLoadContext.Default.LoadFromStream(s));
            Check(s.Position == s.Length, "position");
        }
    }
    public static void Offset()
    {
        Load(Data.Dependency());
        var b = Data.Fixture();
        var all = new byte[b.Length + 9];
        Array.Copy(b, 0, all, 9, b.Length);
        using (var s = new MemoryStream(all))
        {
            s.Position = 9;
            Verify(AssemblyLoadContext.Default.LoadFromStream(s));
            Check(s.Position == s.Length, "position");
        }
    }
    public static void Duplicate()
    {
        Load(Data.Dependency());
        var b = Data.Fixture();
        var a = Load(b);
        Check(Object.ReferenceEquals(a, Load(b)), "same assembly");
        Verify(a);
    }
    public static void Conflict()
    {
        Load(Data.Dependency());
        Load(Data.Fixture());
        bool caught = false;
        try
        {
            Load(Data.Other());
        }
        catch (FileLoadException ex)
        {
            Check(ex.HResult == unchecked((int)0x80131621) && ex.Message.Length > 0, "conflict error details");
            caught = true;
        }
        Check(caught, "conflict");
    }
    public static void Symbols()
    {
        Load(Data.Dependency());
        using (var a = new MemoryStream(Data.Fixture())) using (var p = new MemoryStream(Data.Symbols()))
            Verify(AssemblyLoadContext.Default.LoadFromStream(a, p));
    }
    public static void BadSymbols()
    {
        Load(Data.Dependency());
        using (var a = new MemoryStream(Data.Fixture())) using (var p = new MemoryStream(new byte[] { 1, 2, 3 }))
            Verify(AssemblyLoadContext.Default.LoadFromStream(a, p));
    }
    public static void Forward()
    {
        using (var s = new ForwardOnly(Data.Fixture()))
        {
            bool caught = false;
            try
            {
                AssemblyLoadContext.Default.LoadFromStream(s);
            }
            catch (NotSupportedException)
            {
                caught = true;
            }
            Check(caught, "forward only");
        }
    }
    public static void Invalid()
    {
        bool caught = false;
        try
        {
            Load(new byte[] { 1, 2, 3 });
        }
        catch (BadImageFormatException)
        {
            caught = true;
        }
        Check(caught, "invalid");
        Memory();
    }
    public static void Empty()
    {
        bool caught = false;
        try
        {
            Load(new byte[0]);
        }
        catch (BadImageFormatException)
        {
            caught = true;
        }
        Check(caught, "empty");
        Memory();
    }
    public static void Closed()
    {
        var s = new MemoryStream(Data.Fixture());
        s.Close();
        bool caught = false;
        try
        {
            AssemblyLoadContext.Default.LoadFromStream(s);
        }
        catch (ObjectDisposedException)
        {
            caught = true;
        }
        Check(caught, "closed");
        Memory();
    }
    public static void Null()
    {
        bool caught = false;
        try
        {
            AssemblyLoadContext.Default.LoadFromStream(null);
        }
        catch (ArgumentNullException)
        {
            caught = true;
        }
        Check(caught, "null");
        Memory();
    }
    public static void Lifetime()
    {
        Load(Data.Dependency());
        var b = Data.Fixture();
        var a = Load(b);
        Array.Clear(b, 0, b.Length);
        b = null;
        GC.Collect();
        GC.WaitForPendingFinalizers();
        Verify(a);
    }
    public static void Custom()
    {
        var c = new AssemblyLoadContext("custom", false);
        Check(c != AssemblyLoadContext.Default, "custom");
    }
    public static void Collectible()
    {
        var c = new AssemblyLoadContext("collectible", true);
        c.Unload();
    }
    public static void BeforeDependency()
    {
        var a = Load(Data.Fixture());
        Load(Data.Dependency());
        Verify(a);
    }
    public static void EquivalentImage()
    {
        Load(Data.Dependency());
        var b = Data.Fixture();
        var a = Load(b);
        var padded = new byte[b.Length + 19];
        Array.Copy(b, padded, b.Length);
        Check(Object.ReferenceEquals(a, Load(padded)), "same definition with padding");
    }
    public static void UnsupportedContexts()
    {
        DefaultOnly();
        for (int i = 0; i < 20; i++)
        {
            bool caught = false;
            try
            {
                new AssemblyLoadContext("custom", false);
            }
            catch (NotSupportedException)
            {
                caught = true;
            }
            Check(caught, "custom rejected");
            caught = false;
            try
            {
                new AssemblyLoadContext("collectible", true);
            }
            catch (NotSupportedException)
            {
                caught = true;
            }
            Check(caught, "collectible rejected");
        }
        GC.Collect();
        GC.WaitForPendingFinalizers();
        Memory();
    }
    public static void ShortReads()
    {
        Load(Data.Dependency());
        using (var s = new Chunked(Data.Fixture())) Verify(AssemblyLoadContext.Default.LoadFromStream(s));
    }
    public static void ReadFailure()
    {
        bool caught = false;
        try
        {
            using (var s = new Broken(Data.Fixture())) AssemblyLoadContext.Default.LoadFromStream(s);
        }
        catch (IOException)
        {
            caught = true;
        }
        Check(caught, "read failure");
        Memory();
    }
    public static void FileStream()
    {
        Load(Data.Dependency());
        using (var s = File.OpenRead("out/assembly-stream/fixture/StreamFixture.dll"))
            Verify(AssemblyLoadContext.Default.LoadFromStream(s));
    }
    public static object GetDefaultContext()
    {
        return AssemblyLoadContext.Default;
    }
    public static void InvalidFileStream()
    {
        Load(Data.Dependency());
        bool caught = false;
        try
        {
            using (var s = File.OpenRead("src/tests/assembly-stream/Program.cs"))
                AssemblyLoadContext.Default.LoadFromStream(s);
        }
        catch (BadImageFormatException)
        {
            caught = true;
        }
        Check(caught, "bad file image");
        Memory();
    }
    public static void MissingDependency()
    {
        var a = Load(Data.Fixture());
        bool caught = false;
        try
        {
            a.GetType("StreamFixture.Entry").GetMethod("Answer").Invoke(null, null);
        }
        catch (TargetInvocationException e)
        {
            caught = e.InnerException is FileNotFoundException;
        }
        catch (FileNotFoundException)
        {
            caught = true;
        }
        Check(caught, "missing dependency");
        Load(Data.Dependency());
        Verify(a);
    }
    public static void Truncated()
    {
        var bytes = Data.Fixture();
        foreach (int n in new int[] { 0, 1, 63, 128, 256, 512, 1024 })
        {
            var b = new byte[n];
            Array.Copy(bytes, b, n);
            bool caught = false;
            try
            {
                Load(b);
            }
            catch (BadImageFormatException)
            {
                caught = true;
            }
            Check(caught, "truncated image");
        }
        Memory();
    }
    public static void Main(string[] args)
    {
        typeof(Program).GetMethod(args[0]).Invoke(null, null);
        Console.WriteLine("PASS " + args[0]);
    }
}
}
