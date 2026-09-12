using System.Reflection;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;

[assembly: ManagedNet10.Smoke.MetadataOnlyAttribute("assembly-data", 19, Label = "assembly", Level = 21)]
[module: ManagedNet10.Smoke.MetadataOnlyAttribute("module-data", 23, Label = "module", Level = 25)]

namespace ManagedNet10.Smoke;

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Method)]
internal sealed class SmokeAttribute(string name) : Attribute
{
    public string Name { get; } = name;
}

[AttributeUsage(
    AttributeTargets.Assembly
    | AttributeTargets.Module
    | AttributeTargets.Class
    | AttributeTargets.Constructor
    | AttributeTargets.Field
    | AttributeTargets.Method
    | AttributeTargets.Parameter
    | AttributeTargets.Property
    | AttributeTargets.Event)]
internal sealed class MetadataOnlyAttribute(string text, int number) : Attribute
{
    public string Text { get; } = text;

    public int Number { get; } = number;

    public string Label { get; set; } = "";

    public int Level;
}

internal enum BlobShapeKind
{
    None = 0,
    Primary = 2,
}

[AttributeUsage(AttributeTargets.Class)]
internal sealed class BlobShapeAttribute(
    BlobShapeKind kind,
    Type targetType,
    int[] numbers,
    Type[] types,
    object boxed,
    object[] values) : Attribute
{
    public BlobShapeKind Kind { get; } = kind;

    public Type TargetType { get; } = targetType;

    public int[] Numbers { get; } = numbers;

    public Type[] Types { get; } = types;

    public object Boxed { get; } = boxed;

    public object[] Values { get; } = values;

    public BlobShapeKind NamedKind { get; set; }

    public Type? NamedType { get; set; }

    public int[] Scores { get; set; } = [];

    public object? NamedObject { get; set; }
}

[Smoke("payload")]
internal sealed record class Payload<T>(T Value)
{
    private readonly string _secret = "leanclr";

    public string Secret => _secret;
}

internal sealed class ReflectionInvokeProbe(string name, int number)
{
    public string Name { get; } = name;

    public int Number { get; } = number;

    public string Combine(string suffix, int delta)
    {
        return $"{Name}:{suffix}:{Number + delta}";
    }
}

[MetadataOnly("payload-data", 7, Label = "named", Level = 3)]
internal sealed class CustomAttributeDataProbe
{
    [MetadataOnly("ctor-data", 35, Label = "ctor", Level = 37)]
    public CustomAttributeDataProbe()
    {
    }

    [MetadataOnly("field-data", 11, Label = "field", Level = 5)]
    public int Data = 0;

    [MetadataOnly("property-data", 15, Label = "property", Level = 17)]
    public int Count { get; set; }

    [MetadataOnly("event-data", 27, Label = "event", Level = 29)]
    public event EventHandler? Changed
    {
        add { }
        remove { }
    }

    [MetadataOnly("method-data", 13, Label = "method", Level = 9)]
    public void Run()
    {
    }

    public void WithParameter([MetadataOnly("parameter-data", 31, Label = "parameter", Level = 33)] int value)
    {
    }
}

[BlobShape(
    BlobShapeKind.Primary,
    typeof(Payload<int>),
    new[] { 1, 2, 3 },
    new[] { typeof(string), typeof(Pair) },
    42,
    new object[] { "odin", 9, BlobShapeKind.Primary, typeof(Program) },
    NamedKind = BlobShapeKind.Primary,
    NamedType = typeof(CustomAttributeDataProbe),
    Scores = new[] { 4, 5 },
    NamedObject = "named-object")]
internal sealed class CustomAttributeBlobShapeProbe
{
}

internal readonly record struct Pair(int Left, int Right);

[StructLayout(LayoutKind.Explicit, Pack = 2, Size = 32, CharSet = CharSet.Unicode)]
internal struct ExplicitLayoutProbe
{
    [FieldOffset(0)]
    public int Left;

    [FieldOffset(8)]
    public short Right;
}

[InlineArray(16)]
internal struct InlineIntBuffer
{
    private int _element0;
}

internal struct InlineArrayHolder
{
    public int Length;
    public InlineIntBuffer Buffer;
    public int[]? Large;
}

internal static class Program
{
    private const int ConstNumber = 1234;
    private const string ConstText = "leanclr-const";
    private const char ConstMarker = 'C';

    private static Type? s_seenType;

    private static async Task Main()
    {
        TestBasics();
        TestGenericsDelegatesAndExceptions();
        TestReflection();
        TestSpan();
        TestRuntimeHelpers();
        TestCommandLineArguments();
        TestThreadingSubset();
        HostBridgeWrapperSmoke.Run();
        EngineBindingSmoke.Run();
        ValueMarshalSmoke.Run();
        DiagnosticsSmoke.Run();
        await TestAsync();
    }

    private static void TestBasics()
    {
        TestPairArithmetic();
        TestBoxingMetadata();
    }

    private static void TestPairArithmetic()
    {
        var pair = new Pair(19, 23);
        Require(pair.Left + pair.Right == 42, "record struct arithmetic failed");
    }

    private static void TestBoxingMetadata()
    {
        var pair = new Pair(19, 23);
        object boxed = pair;
        Require(boxed.GetType().Name == nameof(Pair), "boxing metadata failed");
    }

    private static void TestBoxOnly()
    {
        var pair = new Pair(19, 23);
        object boxed = pair;
        Require(boxed != null, "box object failed");
    }

    private static void TestBoxGetTypeOnly()
    {
        var pair = new Pair(19, 23);
        object boxed = pair;
        Require(boxed.GetType() != null, "boxed GetType failed");
    }

    private static void TestBoxGetTypeNoCompare()
    {
        var pair = new Pair(19, 23);
        object boxed = pair;
        boxed.GetType();
    }

    private static void TestTypeOfNoCompare()
    {
        _ = typeof(Pair);
    }

    private static void TestTypeOfKeepAlive()
    {
        s_seenType = typeof(Pair);
        Require(s_seenType != null, "typeof keepalive failed");
    }

    private static void TestTypeOfNameOnly()
    {
        Require(typeof(Pair).Name == nameof(Pair), "typeof metadata name failed");
    }

    private static void TestBoxGetTypeNameOnly()
    {
        var pair = new Pair(19, 23);
        object boxed = pair;
        Require(boxed.GetType().Name == nameof(Pair), "boxed GetType name failed");
    }

    private static void TestGenericsDelegatesAndExceptions()
    {
        var payload = new Payload<int>(21);
        Func<int, int> doubleValue = static value => value * 2;
        Require(doubleValue(payload.Value) == 42, "generic delegate call failed");

        try
        {
            ThrowNested();
            throw new InvalidOperationException("unreachable");
        }
        catch (TargetInvocationException ex) when (ex.InnerException is NotSupportedException)
        {
            Require(ex.InnerException.Message.Contains("smoke", StringComparison.Ordinal), "exception filter failed");
        }
    }

    private static void TestPayloadCtorOnly()
    {
        var payload = new Payload<int>(21);
        Require(payload.Value == 21, "generic payload ctor failed");
    }

    private static void TestStaticDelegateCreateOnly()
    {
        Func<int, int> doubleValue = static value => value * 2;
        Require(doubleValue != null, "static delegate create failed");
    }

    private static void TestStaticDelegateInvokeOnly()
    {
        Func<int, int> doubleValue = static value => value * 2;
        Require(doubleValue(21) == 42, "static delegate invoke failed");
    }

    private static void TestExceptionCtorOnly()
    {
        var ex = new NotSupportedException("smoke");
        Require(ex.Message.Contains("smoke", StringComparison.Ordinal), "exception ctor failed");
    }

    private static void TestExceptionNewOnly()
    {
        var ex = new NotSupportedException();
        Require(ex != null, "exception new failed");
    }

    private static void TestExceptionMessageNewOnly()
    {
        var ex = new NotSupportedException("smoke");
        Require(ex != null, "exception message new failed");
    }

    private static void TestExceptionMessageReadOnly()
    {
        var ex = new NotSupportedException("smoke");
        Require(ex.Message != null, "exception message read failed");
    }

    private static void TestStringContainsOrdinalOnly()
    {
        Require("smoke".Contains("sm", StringComparison.Ordinal), "string contains ordinal failed");
    }

    private static void TestThrowCatchOnly()
    {
        try
        {
            throw new NotSupportedException("smoke");
        }
        catch (NotSupportedException ex)
        {
            Require(ex.Message.Contains("smoke", StringComparison.Ordinal), "exception catch failed");
        }
    }

    private static void TestNestedThrowCatchOnly()
    {
        try
        {
            ThrowNested();
        }
        catch (TargetInvocationException ex)
        {
            Require(ex.InnerException is NotSupportedException, "nested exception catch failed");
        }
    }

    private static void TestTargetInvocationCtorOnly()
    {
        var inner = new NotSupportedException("smoke");
        var ex = new TargetInvocationException(inner);
        Require(ex != null, "target invocation ctor failed");
    }

    private static void TestTargetInvocationMessageCtorOnly()
    {
        var inner = new NotSupportedException("smoke");
        var ex = new TargetInvocationException("wrapped", inner);
        Require(ex != null, "target invocation message ctor failed");
    }

    private static void TestTargetInvocationInnerReadOnly()
    {
        var inner = new NotSupportedException("smoke");
        var ex = new TargetInvocationException(inner);
        Require(ex.InnerException is NotSupportedException, "target invocation inner read failed");
    }

    private static void TestThrowTargetInvocationCatchOnly()
    {
        try
        {
            throw new TargetInvocationException(new NotSupportedException("smoke"));
        }
        catch (TargetInvocationException ex)
        {
            Require(ex.InnerException is NotSupportedException, "target invocation throw catch failed");
        }
    }

    private static void TestExceptionFilterOnly()
    {
        try
        {
            ThrowNested();
            throw new InvalidOperationException("unreachable");
        }
        catch (TargetInvocationException ex) when (ex.InnerException is NotSupportedException)
        {
            Require(ex.InnerException.Message.Contains("smoke", StringComparison.Ordinal), "exception filter failed");
        }
    }

    private static void TestReflection()
    {
        TestAssemblyFullNameOnly();
        TestStructLayoutAttributeOnly();
        TestResolveUserStringOnly();
        TestModuleVersionIdOnly();
        TestParameterDefaultValueOnly();
        TestFieldRawConstantValueOnly();
        TestReflectionInvokeMethodOnly();
        TestCustomAttributeDataOnly();

        var type = typeof(Payload<string>);
        var attr = type.GetCustomAttribute<SmokeAttribute>();
        Require(attr?.Name == "payload", "custom attribute lookup failed");

        var field = type.GetField("_secret", BindingFlags.Instance | BindingFlags.NonPublic);
        Require(field != null, "private field lookup failed");

        var payload = new Payload<string>("value");
        Require((string?)field.GetValue(payload) == "leanclr", "private field read failed");
    }

    private static void TestAssemblyFullNameOnly()
    {
        var fullName = typeof(Program).Assembly.FullName;
        Require(fullName?.Contains("ManagedNet10.Smoke", StringComparison.Ordinal) == true, "assembly full name failed");
    }

    private static void TestStructLayoutAttributeOnly()
    {
        var attr = typeof(ExplicitLayoutProbe).GetCustomAttribute<StructLayoutAttribute>();
        Require(attr != null, "struct layout attribute lookup failed");
        Require(attr.Value == LayoutKind.Explicit, "struct layout kind failed");
        Require(attr.Pack == 2, "struct layout pack failed");
        Require(attr.Size == 32, "struct layout size failed");
        Require(attr.CharSet == CharSet.Unicode, "struct layout charset failed");

        var field = typeof(ExplicitLayoutProbe).GetField(nameof(ExplicitLayoutProbe.Right));
        var fieldOffset = field?.GetCustomAttribute<FieldOffsetAttribute>();
        Require(fieldOffset?.Value == 8, "field offset attribute lookup failed");
    }

    private static void TestResolveUserStringOnly()
    {
        var method = typeof(Program).GetMethod(nameof(UserStringLiteral), BindingFlags.Static | BindingFlags.NonPublic);
        var il = method?.GetMethodBody()?.GetILAsByteArray();
        Require(il != null, "user string method body lookup failed");

        int token = 0;
        for (int i = 0; i + 4 < il.Length; i++)
        {
            if (il[i] == 0x72)
            {
                token = BitConverter.ToInt32(il, i + 1);
                break;
            }
        }

        Require(token != 0, "ldstr token lookup failed");
        Require(method!.Module.ResolveString(token) == "leanclr-user-string", "module user string resolve failed");
    }

    private static string UserStringLiteral()
    {
        return "leanclr-user-string";
    }

    private static void TestModuleVersionIdOnly()
    {
        Require(typeof(Program).Module.ModuleVersionId != Guid.Empty, "module version id lookup failed");
    }

    private static void TestParameterDefaultValueOnly()
    {
        var method = typeof(Program).GetMethod(nameof(DefaultValueProbe), BindingFlags.Static | BindingFlags.NonPublic);
        Require(method != null, "parameter default method lookup failed");
        var parameters = method!.GetParameters();
        Require(parameters.Length == 4, "parameter default count failed");

        Require(parameters[0].DefaultValue is int number && number == 42, "int parameter default value failed");
        Require((string?)parameters[1].DefaultValue == "leanclr-default", "string parameter default value failed");
        Require(parameters[2].DefaultValue == null, "null parameter default value failed");
        Require(parameters[3].DefaultValue is char marker && marker == 'L', "char parameter default value failed");
    }

    private static void DefaultValueProbe(int number = 42, string text = "leanclr-default", object? optional = null, char marker = 'L')
    {
    }

    private static void TestFieldRawConstantValueOnly()
    {
        const BindingFlags flags = BindingFlags.Static | BindingFlags.NonPublic;
        Require(typeof(Program).GetField(nameof(ConstNumber), flags)?.GetRawConstantValue() is int number && number == ConstNumber,
            "int field raw constant value failed");
        Require((string?)typeof(Program).GetField(nameof(ConstText), flags)?.GetRawConstantValue() == ConstText,
            "string field raw constant value failed");
        Require(typeof(Program).GetField(nameof(ConstMarker), flags)?.GetRawConstantValue() is char marker && marker == ConstMarker,
            "char field raw constant value failed");
    }

    private static void TestReflectionInvokeMethodOnly()
    {
        const BindingFlags staticFlags = BindingFlags.Static | BindingFlags.NonPublic;
        var staticMethod = typeof(Program).GetMethod(nameof(ReflectionInvokeJoin), staticFlags);
        Require(staticMethod != null, "reflection static invoke method lookup failed");
        Require((string?)staticMethod!.Invoke(null, new object?[] { "leanclr", 10 }) == "leanclr:20", "reflection static invoke failed");

        var ctor = typeof(ReflectionInvokeProbe).GetConstructor(new Type[] { typeof(string), typeof(int) });
        Require(ctor != null, "reflection constructor lookup failed");
        var probe = (ReflectionInvokeProbe)ctor!.Invoke(new object?[] { "probe", 7 });
        Require(probe.Name == "probe" && probe.Number == 7, "reflection constructor invoke failed");

        var instanceMethod = typeof(ReflectionInvokeProbe).GetMethod(nameof(ReflectionInvokeProbe.Combine));
        Require(instanceMethod != null, "reflection instance invoke method lookup failed");
        Require((string?)instanceMethod!.Invoke(probe, new object?[] { "suffix", 5 }) == "probe:suffix:12", "reflection instance invoke failed");
    }

    private static string ReflectionInvokeJoin(string prefix, int number)
    {
        return $"{prefix}:{number * 2}";
    }

    private static void TestCustomAttributeDataOnly()
    {
        var assemblyAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(typeof(Program).Assembly));
        ValidateMetadataOnlyAttributeData(assemblyAttr, "assembly-data", 19, "assembly", 21);
        var assemblyInstanceAttr = FindMetadataOnlyAttributeData(typeof(Program).Assembly.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(assemblyInstanceAttr, "assembly-data", 19, "assembly", 21);

        var moduleAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(typeof(Program).Module));
        ValidateMetadataOnlyAttributeData(moduleAttr, "module-data", 23, "module", 25);
        var moduleInstanceAttr = FindMetadataOnlyAttributeData(typeof(Program).Module.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(moduleInstanceAttr, "module-data", 23, "module", 25);

        var attributes = CustomAttributeData.GetCustomAttributes(typeof(CustomAttributeDataProbe));
        var metadataAttr = FindMetadataOnlyAttributeData(attributes);
        ValidateMetadataOnlyAttributeData(metadataAttr, "payload-data", 7, "named", 3);
        var typeInstanceAttr = FindMetadataOnlyAttributeData(typeof(CustomAttributeDataProbe).GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(typeInstanceAttr, "payload-data", 7, "named", 3);

        var field = typeof(CustomAttributeDataProbe).GetField(nameof(CustomAttributeDataProbe.Data));
        Require(field != null, "custom attribute data field target lookup failed");
        var fieldAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(field!));
        ValidateMetadataOnlyAttributeData(fieldAttr, "field-data", 11, "field", 5);
        var fieldInstanceAttr = FindMetadataOnlyAttributeData(field!.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(fieldInstanceAttr, "field-data", 11, "field", 5);

        var method = typeof(CustomAttributeDataProbe).GetMethod(nameof(CustomAttributeDataProbe.Run));
        Require(method != null, "custom attribute data method target lookup failed");
        var methodAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(method!));
        ValidateMetadataOnlyAttributeData(methodAttr, "method-data", 13, "method", 9);
        var methodInstanceAttr = FindMetadataOnlyAttributeData(method!.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(methodInstanceAttr, "method-data", 13, "method", 9);

        var parameterMethod = typeof(CustomAttributeDataProbe).GetMethod(nameof(CustomAttributeDataProbe.WithParameter));
        Require(parameterMethod != null, "custom attribute data parameter method lookup failed");
        var parameters = parameterMethod!.GetParameters();
        Require(parameters.Length == 1, "custom attribute data parameter target lookup failed");
        var parameterAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(parameters[0]));
        ValidateMetadataOnlyAttributeData(parameterAttr, "parameter-data", 31, "parameter", 33);
        var parameterInstanceAttr = FindMetadataOnlyAttributeData(parameters[0].GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(parameterInstanceAttr, "parameter-data", 31, "parameter", 33);

        var property = typeof(CustomAttributeDataProbe).GetProperty(nameof(CustomAttributeDataProbe.Count));
        Require(property != null, "custom attribute data property target lookup failed");
        var propertyAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(property!));
        ValidateMetadataOnlyAttributeData(propertyAttr, "property-data", 15, "property", 17);
        var propertyInstanceAttr = FindMetadataOnlyAttributeData(property!.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(propertyInstanceAttr, "property-data", 15, "property", 17);

        var eventInfo = typeof(CustomAttributeDataProbe).GetEvent(nameof(CustomAttributeDataProbe.Changed));
        Require(eventInfo != null, "custom attribute data event target lookup failed");
        var eventAttr = FindMetadataOnlyAttributeData(CustomAttributeData.GetCustomAttributes(eventInfo!));
        ValidateMetadataOnlyAttributeData(eventAttr, "event-data", 27, "event", 29);
        var eventInstanceAttr = FindMetadataOnlyAttributeData(eventInfo!.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(eventInstanceAttr, "event-data", 27, "event", 29);

        var constructor = typeof(CustomAttributeDataProbe).GetConstructor(Type.EmptyTypes);
        Require(constructor != null, "custom attribute data constructor target lookup failed");
        var constructorInstanceAttr = FindMetadataOnlyAttributeData(constructor!.GetCustomAttributesData());
        ValidateMetadataOnlyAttributeData(constructorInstanceAttr, "ctor-data", 35, "ctor", 37);

        TestCustomAttributeDataBlobShapesOnly();
    }

    private static CustomAttributeData? FindMetadataOnlyAttributeData(IList<CustomAttributeData> attributes)
    {
        for (int i = 0; i < attributes.Count; i++)
        {
            if (attributes[i].AttributeType == typeof(MetadataOnlyAttribute))
            {
                return attributes[i];
            }
        }

        return null;
    }

    private static void ValidateMetadataOnlyAttributeData(CustomAttributeData? metadataAttr, string text, int number, string label, int level)
    {
        Require(metadataAttr != null, "custom attribute data lookup failed");
        Require(metadataAttr!.Constructor.DeclaringType == typeof(MetadataOnlyAttribute), "custom attribute data constructor failed");

        var ctorArgs = metadataAttr.ConstructorArguments;
        Require(ctorArgs.Count == 2, "custom attribute data ctor arg count failed");
        Require((string?)ctorArgs[0].Value == text, "custom attribute data string ctor arg failed");
        Require(ctorArgs[1].Value is int actualNumber && actualNumber == number, "custom attribute data int ctor arg failed");

        bool sawLabel = false;
        bool sawLevel = false;
        var namedArgs = metadataAttr.NamedArguments;
        for (int i = 0; i < namedArgs.Count; i++)
        {
            var namedArg = namedArgs[i];
            if (namedArg.MemberName == nameof(MetadataOnlyAttribute.Label))
            {
                sawLabel = (string?)namedArg.TypedValue.Value == label;
            }
            else if (namedArg.MemberName == nameof(MetadataOnlyAttribute.Level))
            {
                sawLevel = namedArg.TypedValue.Value is int actualLevel && actualLevel == level;
            }
        }

        Require(sawLabel, "custom attribute data property named arg failed");
        Require(sawLevel, "custom attribute data field named arg failed");
    }

    private static void TestCustomAttributeDataBlobShapesOnly()
    {
        var attributes = CustomAttributeData.GetCustomAttributes(typeof(CustomAttributeBlobShapeProbe));
        var blobAttr = FindAttributeData(attributes, typeof(BlobShapeAttribute));
        Require(blobAttr != null, "custom attribute data blob-shape lookup failed");

        var ctorArgs = blobAttr!.ConstructorArguments;
        Require(ctorArgs.Count == 6, "custom attribute data blob-shape ctor arg count failed");
        Require(ctorArgs[0].ArgumentType == typeof(BlobShapeKind), "custom attribute data enum arg type failed");
        Require(ctorArgs[0].Value is int kind && kind == (int)BlobShapeKind.Primary, "custom attribute data enum arg value failed");
        Require(ctorArgs[1].Value is Type targetType && targetType == typeof(Payload<int>), "custom attribute data Type arg failed");
        ValidateIntTypedArgumentArray(ctorArgs[2], [1, 2, 3]);
        ValidateTypeTypedArgumentArray(ctorArgs[3], [typeof(string), typeof(Pair)]);
        Require(ctorArgs[4].ArgumentType == typeof(int), "custom attribute data object arg encoded type failed");
        Require(ctorArgs[4].Value is int boxed && boxed == 42, "custom attribute data object arg value failed");
        ValidateObjectTypedArgumentArray(ctorArgs[5]);

        bool sawNamedKind = false;
        bool sawNamedType = false;
        bool sawScores = false;
        bool sawNamedObject = false;
        var namedArgs = blobAttr.NamedArguments;
        for (int i = 0; i < namedArgs.Count; i++)
        {
            var namedArg = namedArgs[i];
            if (namedArg.MemberName == nameof(BlobShapeAttribute.NamedKind))
            {
                sawNamedKind = namedArg.TypedValue.Value is int namedKind && namedKind == (int)BlobShapeKind.Primary;
            }
            else if (namedArg.MemberName == nameof(BlobShapeAttribute.NamedType))
            {
                sawNamedType = namedArg.TypedValue.Value is Type namedType && namedType == typeof(CustomAttributeDataProbe);
            }
            else if (namedArg.MemberName == nameof(BlobShapeAttribute.Scores))
            {
                ValidateIntTypedArgumentArray(namedArg.TypedValue, [4, 5]);
                sawScores = true;
            }
            else if (namedArg.MemberName == nameof(BlobShapeAttribute.NamedObject))
            {
                sawNamedObject = (string?)namedArg.TypedValue.Value == "named-object";
            }
        }

        Require(sawNamedKind, "custom attribute data named enum arg failed");
        Require(sawNamedType, "custom attribute data named Type arg failed");
        Require(sawScores, "custom attribute data named array arg failed");
        Require(sawNamedObject, "custom attribute data named object arg failed");
    }

    private static CustomAttributeData? FindAttributeData(IList<CustomAttributeData> attributes, Type attributeType)
    {
        for (int i = 0; i < attributes.Count; i++)
        {
            if (attributes[i].AttributeType == attributeType)
            {
                return attributes[i];
            }
        }

        return null;
    }

    private static void ValidateIntTypedArgumentArray(CustomAttributeTypedArgument arg, int[] expected)
    {
        var values = arg.Value as IList<CustomAttributeTypedArgument>;
        Require(values != null, "custom attribute data int array value failed");
        Require(values.Count == expected.Length, "custom attribute data int array length failed");
        for (int i = 0; i < expected.Length; i++)
        {
            Require(values[i].Value is int value && value == expected[i], "custom attribute data int array element failed");
        }
    }

    private static void ValidateTypeTypedArgumentArray(CustomAttributeTypedArgument arg, Type[] expected)
    {
        var values = arg.Value as IList<CustomAttributeTypedArgument>;
        Require(values != null, "custom attribute data Type array value failed");
        Require(values.Count == expected.Length, "custom attribute data Type array length failed");
        for (int i = 0; i < expected.Length; i++)
        {
            Require(values[i].Value is Type type && type == expected[i], "custom attribute data Type array element failed");
        }
    }

    private static void ValidateObjectTypedArgumentArray(CustomAttributeTypedArgument arg)
    {
        var values = arg.Value as IList<CustomAttributeTypedArgument>;
        Require(values != null, "custom attribute data object array value failed");
        Require(values.Count == 4, "custom attribute data object array length failed");
        Require((string?)values[0].Value == "odin", "custom attribute data object array string failed");
        Require(values[1].Value is int number && number == 9, "custom attribute data object array int failed");
        Require(values[2].Value is int kind && kind == (int)BlobShapeKind.Primary, "custom attribute data object array enum failed");
        Require(values[3].Value is Type type && type == typeof(Program), "custom attribute data object array Type failed");
    }

    private static void TestReflectionTypeOnly()
    {
        var type = typeof(Payload<string>);
        Require(type != null, "reflection typeof failed");
    }

    private static void TestReflectionAttributeOnly()
    {
        var type = typeof(Payload<string>);
        var attr = type.GetCustomAttribute<SmokeAttribute>();
        Require(attr != null, "custom attribute object failed");
    }

    private static void TestReflectionAttributeNameOnly()
    {
        var type = typeof(Payload<string>);
        var attr = type.GetCustomAttribute<SmokeAttribute>();
        Require(attr?.Name == "payload", "custom attribute name failed");
    }

    private static void TestAttributeCtorOnly()
    {
        var attr = new SmokeAttribute("payload");
        Require(attr.Name == "payload", "custom attribute ctor failed");
    }

    private static void TestAttributeSubclassOnly()
    {
        Require(typeof(SmokeAttribute).IsSubclassOf(typeof(Attribute)), "attribute subclass check failed");
    }

    private static void TestTypeEqualityOnly()
    {
        Require(typeof(SmokeAttribute) == typeof(SmokeAttribute), "type equality failed");
    }

    private static void TestAttributeBaseTypeOnly()
    {
        Require(typeof(SmokeAttribute).BaseType != null, "attribute base type lookup failed");
    }

    private static void TestAttributeBaseTypeCompareOnly()
    {
        Require(typeof(SmokeAttribute).BaseType == typeof(Attribute), "attribute base type compare failed");
    }

    private static void TestReflectionAttributeArrayOnly()
    {
        var type = typeof(Payload<string>);
        var attrs = type.GetCustomAttributes(typeof(SmokeAttribute), inherit: true);
        Require(attrs.Length == 1, "custom attribute array lookup failed");
    }

    private static void TestReflectionFieldLookupOnly()
    {
        var type = typeof(Payload<string>);
        var field = type.GetField("_secret", BindingFlags.Instance | BindingFlags.NonPublic);
        Require(field != null, "private field lookup failed");
    }

    private static void TestReflectionFieldValueOnly()
    {
        var type = typeof(Payload<string>);
        var field = type.GetField("_secret", BindingFlags.Instance | BindingFlags.NonPublic);
        var payload = new Payload<string>("value");
        Require((string?)field?.GetValue(payload) == "leanclr", "private field read failed");
    }

    private static void TestSpan()
    {
        TestSpanStackalloc();
        TestSpanStackallocInitializer();
        TestSpanBoolStackalloc();
        TestInlineArrayStructLayout();
    }

    private static void TestSpanStackalloc()
    {
        Span<int> values = stackalloc int[6];
        values[0] = 42;
        Require(values[0] == 42, "span stackalloc failed");
    }

    private static void TestSpanStackallocInitializer()
    {
        Span<int> values = stackalloc[] { 4, 8, 15, 16, 23, 42 };
        values[0] = values[^1];
        Require(values[0] == 42, "span index-from-end failed");
    }

    private static void TestSpanBoolStackalloc()
    {
        Span<bool> values = stackalloc bool[2];
        values[1] = true;
        Require(values[1], "span bool stackalloc failed");
        Require(ReadSpanBoolArg(values), "span bool argument read failed");
    }

    private static bool ReadSpanBoolArg(Span<bool> values)
    {
        return values[1];
    }

    private static void TestInlineArrayStructLayout()
    {
        var holder = new InlineArrayHolder();
        holder.Length = 16;
        holder.Buffer[15] = 42;
        Require(holder.Large == null, "inline array tail reference overlapped");

        holder.Large = [7, 42];
        Require(holder.Buffer[15] == 42, "inline array element was clobbered");
        Require(holder.Large[1] == 42, "inline array tail reference read failed");
    }

    private static void TestRuntimeHelpers()
    {
        RuntimeHelpers.EnsureSufficientExecutionStack();
        RuntimeHelpers.TryEnsureSufficientExecutionStack();

        var method = typeof(Program).GetMethod(nameof(RuntimeHelpersPrepareTarget), BindingFlags.Static | BindingFlags.NonPublic);
        Require(method != null, "runtime helper prepare target lookup failed");
        RuntimeHelpers.PrepareMethod(method!.MethodHandle);
        Require(RuntimeHelpersPrepareTarget(40, 2) == 42, "runtime helper prepared target failed");
    }

    private static int RuntimeHelpersPrepareTarget(int left, int right)
    {
        return left + right;
    }

    private static void TestCommandLineArguments()
    {
        string[] args = Environment.GetCommandLineArgs();
        Require(args.Length >= 1, "command line args missing argv0");
        Require(args[0].Contains("ManagedNet10.Smoke", StringComparison.Ordinal), "command line argv0 assembly name failed");
    }

    private static void TestThreadingSubset()
    {
        var value = 0;
        Require(Interlocked.Increment(ref value) == 1, "interlocked increment failed");
        Volatile.Write(ref value, 41);
        Require(Volatile.Read(ref value) == 41, "volatile read/write failed");
    }

    private static void TestCurrentThreadOnly()
    {
        Require(Thread.CurrentThread != null, "current thread lookup failed");
    }

    private static void TestTaskFromResultOnly()
    {
        var task = Task.FromResult(42);
        Require(task != null, "Task.FromResult returned null");
    }

    private static void TestTaskFromResultValueOnly()
    {
        Require(Task.FromResult(42).Result == 42, "Task.FromResult value failed");
    }

    private static void TestReturnsResultAsyncOnly()
    {
        Require(ReturnsResultAsync().Result == 42, "ReturnsResultAsync value failed");
    }

    private static void TestYieldAwaiterOnly()
    {
        var awaiter = Task.Yield().GetAwaiter();
        awaiter.GetResult();
    }

    private static void TestAsyncTaskMethodBuilderTaskOnly()
    {
        var builder = AsyncTaskMethodBuilder.Create();
        Require(builder.Task != null, "async builder task failed");
    }

    private static void TestAsyncTaskMethodBuilderSetResultOnly()
    {
        var builder = AsyncTaskMethodBuilder.Create();
        builder.SetResult();
        Require(builder.Task != null, "async builder set result failed");
    }

    private static void TestAsyncNoAwaitCreateOnly()
    {
        var task = AsyncNoAwait();
        Require(task != null, "async no-await method returned null");
    }

    private static void TestAsyncCompletedAwaitCreateOnly()
    {
        var task = AsyncCompletedAwait();
        Require(task != null, "async completed-await method returned null");
    }

    private static void TestAsyncCreateOnly()
    {
        var task = TestAsync();
        Require(task != null, "async method returned null");
    }

    private static async Task AsyncNoAwait()
    {
        Require(true, "async no-await body failed");
    }

    private static async Task AsyncCompletedAwait()
    {
        Require(await ReturnsResultAsync() == 42, "async completed-await result failed");
    }

    private static async Task TestAsync()
    {
        await Task.Yield();
        Require(await ReturnsResultAsync() == 42, "async result failed");
    }

    private static Task<int> ReturnsResultAsync()
    {
        return Task.FromResult(42);
    }

    private static void ThrowNested()
    {
        try
        {
            throw new NotSupportedException("smoke");
        }
        catch (NotSupportedException ex)
        {
            throw new TargetInvocationException(ex);
        }
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }
}
