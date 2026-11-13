using System.Globalization;
namespace ManagedNet10.GlobalizationContracts;
public static class Program
{
    static void Check(bool value, string message) { if (!value) throw new Exception(message); }
    public static void Strings()
    {
        Check("__BoolValue".StartsWith("__") && "__X_Attrs".EndsWith("_Attrs"), "culture marker lookup");
        Check("abcabc".IndexOf("bc") == 1 && "abcabc".LastIndexOf("bc") == 4, "culture find");
        Check("abc".StartsWith("") && "abc".EndsWith("") && !"a".StartsWith("abc"), "empty and long needles");
        Check(string.Compare("a", "b") < 0 && string.Compare("b", "a") > 0, "comparison symmetry");
        Check(string.Compare(new string('x', 2), new string('x', 2)) == 0, "comparison equality");
        Check("abc".ToUpper() == "ABC" && "ABC".ToLower() == "abc", "culture case");
        Check("\u00e4\u03c3".ToUpperInvariant() == "\u00c4\u03a3", "Unicode invariant casing");
        Check("\U00010428".ToUpperInvariant() == "\U00010400", "supplementary invariant casing");
        Check("\u00e4".Equals("\u00c4", StringComparison.OrdinalIgnoreCase), "Unicode ordinal casing");
        Console.WriteLine("PASS Strings");
    }
    public static void InvariantProfile()
    {
        Check(CultureInfo.CurrentCulture.Name == "", "minimal non-Windows culture must be invariant");
        Check(12.5.ToString(CultureInfo.InvariantCulture) == "12.5", "invariant numbers");
        Check(new DateTime(2026, 11, 13).ToString("yyyy-MM-dd", CultureInfo.InvariantCulture) == "2026-11-13", "invariant date");
        try { _ = new CultureInfo("tr-TR"); throw new Exception("unsupported culture accepted"); }
        catch (CultureNotFoundException) { }
        Console.WriteLine("PASS InvariantProfile");
    }
    public static void EnvironmentRoundtrip()
    {
        const string name = "LEANCLR_GLOBALIZATION_ENV_TEST";
        try
        {
            Environment.SetEnvironmentVariable(name, "value-\u4e2d\U0001f600");
            Check(Environment.GetEnvironmentVariable(name) == "value-\u4e2d\U0001f600", "environment UTF roundtrip");
            Environment.SetEnvironmentVariable(name, new string('x', 1024));
            Check(Environment.GetEnvironmentVariable(name)!.Length == 1024, "large environment value");
            Environment.SetEnvironmentVariable(name, null);
            Check(Environment.GetEnvironmentVariable(name) == null, "missing environment variable");
        }
        finally { Environment.SetEnvironmentVariable(name, null); }
        Console.WriteLine("PASS EnvironmentRoundtrip");
    }
    public static void Main(string[] args)
    {
        Strings();
        if (args.Length > 0 && args[0] == "invariant") InvariantProfile();
    }
}
