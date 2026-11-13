using System;
using System.IO;
using System.Text;
namespace ConsoleOutputTests
{
public class Sink : StringWriter
{
    public int Lines;
    public override void WriteLine(string value)
    {
        Lines++;
        base.WriteLine(value);
    }
}
public class BrokenWriter : StringWriter
{
    public override void WriteLine(string value)
    {
        throw new InvalidOperationException("writer error");
    }
}
public class Program
{
    static void Check(bool b, string message)
    {
        if (!b)
            throw new Exception(message);
    }
    public static void Redirect()
    {
        TextWriter original = Console.Out;
        var w = new Sink();
        try
        {
            Console.SetOut(w);
            Console.WriteLine("capture");
            Console.WriteLine(123);
            Console.WriteLine((string)null);
            Console.Write("tail");
        }
        finally
        {
            Console.SetOut(original);
        }
        Check(w.ToString() ==
                  "capture" + Environment.NewLine + "123" + Environment.NewLine + Environment.NewLine + "tail",
              "redirect all overloads");
        Check(w.Lines >= 2, "virtual string writer");
    }
    public static void Unicode()
    {
        var w = new StringWriter();
        TextWriter original = Console.Out;
        try
        {
            Console.SetOut(w);
            Console.WriteLine("\u4e2d\u6587\0end");
        }
        finally
        {
            Console.SetOut(original);
        }
        Check(w.ToString() == "\u4e2d\u6587\0end" + Environment.NewLine, "unicode and embedded null");
    }
    public static void Error()
    {
        var w = new StringWriter();
        TextWriter original = Console.Error;
        try
        {
            Console.SetError(w);
            Console.Error.WriteLine("error");
        }
        finally
        {
            Console.SetError(original);
        }
        Check(w.ToString() == "error" + Environment.NewLine, "stderr");
    }
    public static void Throw()
    {
        TextWriter original = Console.Out;
        bool caught = false;
        try
        {
            Console.SetOut(new BrokenWriter());
            Console.WriteLine("error");
        }
        catch (InvalidOperationException)
        {
            caught = true;
        }
        finally
        {
            Console.SetOut(original);
        }
        Check(caught, "writer exception");
    }
    public static void Restore()
    {
        TextWriter original = Console.Out;
        var first = new StringWriter();
        var second = new StringWriter();
        try
        {
            Console.SetOut(first);
            Console.WriteLine("first");
            Console.SetOut(second);
            Console.WriteLine("second");
            Console.SetOut(first);
            Console.WriteLine("again");
        }
        finally
        {
            Console.SetOut(original);
        }
        Check(first.ToString() == "first" + Environment.NewLine + "again" + Environment.NewLine &&
                  second.ToString() == "second" + Environment.NewLine,
              "replace and restore");
    }
    public static void Default()
    {
        Console.WriteLine("console-default-string");
        Console.WriteLine(123);
        Console.Out.Flush();
    }
    public static void Main(string[] args)
    {
        typeof(Program).GetMethod(args[0]).Invoke(null, null);
    }
}
}
