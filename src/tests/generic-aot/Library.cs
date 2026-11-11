public static class GenericLibrary
{
    public static T Identity<T>(T value) { return value; }
    public static object Reference(object value) { return value; }
    public static void Touch(int divisor) { if (12 / divisor == 0) throw new System.Exception(); }
}
