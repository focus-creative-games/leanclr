using System;
using System.Text;

class TC_String
{
    [UnitTest]
    public void EmptyString()
    {
        string s = string.Empty;
        Assert.NotNull(s);
        Assert.Equal(0, s.Length);
    }

    public static string Run3()
    {
        char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
        return new string(arr);
    }

    public static string Run4()
    {
        char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
        return new string(arr, 1, 2);
    }

    public unsafe static string Run5()
    {
        char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
        fixed (char* p = &arr[0])
        {
            return new string(p);
        }
    }

    public unsafe static string Run6()
    {
        char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
        fixed (char* p = &arr[0])
        {
            return new string(p, 1, 2);
        }
    }

    public unsafe static string Run7()
    {
        var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
        fixed (sbyte* p = &arr[0])
        {
            return new string(p);
        }
    }

    public unsafe static string Run8()
    {
        var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
        fixed (sbyte* p = &arr[0])
        {
            return new string(p, 1, 2);
        }
    }

    public unsafe static string Run9()
    {
        var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
        fixed (sbyte* p = &arr[0])
        {
            // this function will be redirected to string.Ctor(sbyte*, int, int, Encoding)
            return new string(p, 1, 2, Encoding.UTF8);
        }
    }

    //public unsafe static string Run10()
    //{
    //    char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
    //    return new string(new ReadOnlySpan<char>(arr));
    //}
}

