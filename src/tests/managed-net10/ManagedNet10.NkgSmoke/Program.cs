using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.ExceptionServices;
using System.Reflection;
using System.Text;

namespace ManagedNet10.NkgSmoke;

internal static class Program
{
    private static void Main()
    {
        RunFullWorkloadSurfaceSmoke();
    }

    public static void RunFullWorkloadSurfaceSmoke()
    {
        RunCoreWorkloadSurfaceSmoke();

        RunOdinSerializerPrimitiveProbe();
        RunOdinSerializerConstructionProbe();
        RunOdinSerializerConstructionWithExplicitPolicyProbe();
        RunOdinSerializerUninitializedObjectProbe();
        RunOdinSerializerPrivateFieldSetProbe();
        RunOdinSerializerPrivateFormatFieldSetProbe();
        RunOdinSerializerPrivateFormatJsonFieldSetProbe();
        RunOdinSerializerPrivatePolicyFieldSetProbe();
        RunOdinSerializerPrivateLoggingFieldSetProbe();
        RunOdinSerializerPrivateLoggingZeroFieldSetProbe();
        RunOdinSerializerPrivateErrorHandlingFieldSetProbe();
        RunOdinSerializerConstructorMetadataProbe();
        RunOdinSerializerEnumValueProbe();
        RunOdinSerializerEnumTypeEqualityProbe();
        RunOdinSerializerEnumIsInstanceProbe();
        RunOdinCustomSerializationPolicyConstructionProbe();
        RunOdinSerializationPoliciesEverythingProbe();
        RunOdinSerializerPrimitiveSerializeProbe();
        RunOdinSerializationUtilityWeakPrimitiveProbe();
        RunOdinSerializationUtilityWeakSnapshotProbe();

        RunGodotPlaneWebDebugSmoke();
        RunSamplerOdinRoundTripProbe();

        RunAssemblyNameSmoke();
        RunReferencedAssembliesSmoke();
        RunTypesSmoke();
        RunTypeNamespaceSmoke();
        RunTypeCustomAttributeDataSmoke();
        RunMemberEnumerationSmoke();
        RunMemberCustomAttributeDataSmoke();
        RunFieldCustomAttributeDataSmoke();
        RunPropertyCustomAttributeDataSmoke();
        RunMethodCustomAttributeDataSmoke();
    }

    public static void RunCoreWorkloadSurfaceSmoke()
    {
        RunLeanClrBclCompatibilitySmoke();
        RunReflectionAttributeSmoke();
        RunAsyncAndSerializationSurfaceSmoke();
        RunOdinSerializerRoundTripSmoke();
        RunDiagnosticsWebDebugSmoke();
        RunSamplerCoreGameplaySmoke();
    }

    public static void RunLeanClrBclCompatibilitySmoke()
    {
        RunRuntimeFeatureCompatibilitySmoke();
        RunLinqGenericChainSmoke();
        RunValueTaskCompatibilitySmoke();
        RunExceptionCompatibilitySmoke();
        RunReflectionConstructorEnumNullSmoke();
    }

    public static void RunRuntimeFeatureCompatibilitySmoke()
    {
        Require(!RuntimeFeature.IsDynamicCodeSupported, "LeanCLR must report dynamic code as unsupported");
        Require(!RuntimeFeature.IsDynamicCodeCompiled, "LeanCLR must report dynamic code compilation as unsupported");
        Unsafe.SkipInit(out int skipInitValue);
        skipInitValue = 42;
        Require(skipInitValue == 42, "Unsafe.SkipInit<T> intrinsic failed");
    }

    public static void RunLinqGenericChainSmoke()
    {
        var rows = new[]
        {
            new LinqRow("alpha", 1),
            new LinqRow("alpha", 4),
            new LinqRow("beta", 3),
            new LinqRow("beta", 7),
            new LinqRow("gamma", 0),
        };

        var buckets = rows
            .Where(static row => row.Score >= 2)
            .Select(static row => new LinqRow(row.Group, row.Score + 1))
            .GroupBy(static row => row.Group)
            .Select(static group => new LinqBucket(
                group.Key,
                group.OrderByDescending(static row => row.Score).Select(static row => row.Score).ToArray(),
                group.Sum(static row => row.Score)))
            .OrderBy(static bucket => bucket.Key)
            .ToArray();

        Require(buckets.Length == 2, "LINQ grouped bucket count mismatch");
        Require(buckets[0].Key == "alpha" &&
            buckets[0].Scores.Length == 1 &&
            buckets[0].Scores[0] == 5, "LINQ alpha bucket mismatch");
        Require(buckets[1].Key == "beta" &&
            buckets[1].Scores.Length == 2 &&
            buckets[1].Scores[0] == 8 &&
            buckets[1].Scores[1] == 4, "LINQ beta bucket mismatch");

        var totals = buckets.ToDictionary(static bucket => bucket.Key, static bucket => bucket.Total);
        Require(totals["alpha"] == 5 && totals["beta"] == 12, "LINQ dictionary totals mismatch");
    }

    public static void RunValueTaskCompatibilitySmoke()
    {
        var valueTask = new ValueTask<int>(42);
        Require(valueTask.GetAwaiter().GetResult() == 42, "ValueTask<T> direct awaiter failed");
        Require(valueTask.AsTask().GetAwaiter().GetResult() == 42, "ValueTask<T>.AsTask failed");

        var voidValueTask = new ValueTask(Task.CompletedTask);
        voidValueTask.GetAwaiter().GetResult();
        voidValueTask.AsTask().GetAwaiter().GetResult();

        var asyncResult = AddValueTaskAsync(19, 23).AsTask().GetAwaiter().GetResult();
        Require(asyncResult == 42, "async ValueTask<T> state machine failed");
    }

    public static void RunExceptionCompatibilitySmoke()
    {
        int finallyCount = 0;
        try
        {
            try
            {
                throw new InvalidOperationException("filtered");
            }
            catch (InvalidOperationException exception) when (exception.Message == "filtered")
            {
                finallyCount++;
                throw;
            }
            finally
            {
                finallyCount++;
            }
        }
        catch (InvalidOperationException exception)
        {
            Require(exception.Message == "filtered", "exception filter/rethrow message mismatch");
        }

        try
        {
            throw new ArgumentException("fallback");
        }
        catch (ArgumentException) when (Never())
        {
            throw new InvalidOperationException("exception filter should not enter false branch");
        }
        catch (ArgumentException exception)
        {
            Require(exception.Message == "fallback", "exception filter fallback mismatch");
        }
        finally
        {
            finallyCount++;
        }

        try
        {
            ExceptionDispatchInfo.Capture(new ApplicationException("captured")).Throw();
        }
        catch (ApplicationException exception)
        {
            Require(exception.Message == "captured", "ExceptionDispatchInfo rethrow mismatch");
        }

        var fileLoad = new FileLoadException(null, "NKGGameFramework.dll");
        Require(!string.IsNullOrWhiteSpace(fileLoad.Message), "FileLoadException message qcall failed");
        Require(finallyCount == 3, "try/catch/finally execution count mismatch");
    }

    public static void RunReflectionConstructorEnumNullSmoke()
    {
        var targetType = typeof(ReflectionEnumNullConstructorTarget);
        var constructor = targetType.GetConstructor(
            BindingFlags.Public | BindingFlags.Instance,
            binder: null,
            [
                typeof(LocalDataFormat),
                typeof(ILocalSerializationPolicy),
                typeof(LocalLoggingPolicy),
                typeof(LocalErrorHandlingPolicy),
            ],
            modifiers: null);
        Require(constructor != null, "reflection enum/null constructor surface missing");

        var target = constructor!.Invoke([
            LocalDataFormat.Binary,
            null!,
            LocalLoggingPolicy.Silent,
            LocalErrorHandlingPolicy.ThrowOnErrors,
        ]);

        Require(target is ReflectionEnumNullConstructorTarget, "reflection enum/null constructor returned wrong instance");
        var typedTarget = (ReflectionEnumNullConstructorTarget)target!;
        Require(typedTarget.Format == LocalDataFormat.Binary, "reflection enum/null constructor enum parameter mismatch");
        Require(typedTarget.Policy == null, "reflection enum/null constructor null reference parameter mismatch");
        Require(typedTarget.Logging == LocalLoggingPolicy.Silent, "reflection enum/null constructor second enum parameter mismatch");
        Require(typedTarget.ErrorHandling == LocalErrorHandlingPolicy.ThrowOnErrors, "reflection enum/null constructor third enum parameter mismatch");
    }

    public static void RunOdinSerializerRoundTripSmoke()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var serializer = CreateOdinGameSerializer(serializerType);

        var snapshot = CreateSamplerSnapshot(out var snapshotType);

        var serializeToBytes = RequireInstanceMethod(serializerType, "SerializeToBytes", parameterCount: 1)
            .MakeGenericMethod(snapshotType);
        var deserializeFromBytes = RequireInstanceMethod(serializerType, "DeserializeFromBytes", parameterCount: 1)
            .MakeGenericMethod(snapshotType);
        var serialize = RequireInstanceMethod(serializerType, "Serialize", parameterCount: 1)
            .MakeGenericMethod(snapshotType);
        var deserialize = RequireInstanceMethod(serializerType, "Deserialize", parameterCount: 1)
            .MakeGenericMethod(snapshotType);
        var serializeToJson = RequireInstanceMethod(serializerType, "SerializeToJson", parameterCount: 1)
            .MakeGenericMethod(snapshotType);
        var deserializeFromJson = RequireInstanceMethod(serializerType, "DeserializeFromJson", parameterCount: 1)
            .MakeGenericMethod(snapshotType);

        var binaryPayload = (byte[])serializeToBytes.Invoke(serializer, [snapshot])!;
        Require(binaryPayload.Length > 0, "Odin binary payload was empty");
        AssertSnapshotRoundTrip(snapshotType, deserializeFromBytes.Invoke(serializer, [binaryPayload]));

        var stringPayload = (string)serialize.Invoke(serializer, [snapshot])!;
        Require(!string.IsNullOrWhiteSpace(stringPayload), "Odin string payload was empty");
        AssertSnapshotRoundTrip(snapshotType, deserialize.Invoke(serializer, [stringPayload]));

        var jsonPayload = (string)serializeToJson.Invoke(serializer, [snapshot])!;
        Require(!string.IsNullOrWhiteSpace(jsonPayload), "Odin JSON payload was empty");
        AssertSnapshotRoundTrip(snapshotType, deserializeFromJson.Invoke(serializer, [jsonPayload]));
    }

    public static void RunDiagnosticsWebDebugSmoke()
    {
        var bridgeType = RequireType(
            Assembly.Load("NKGGameFramework.GodotPlaneSample"),
            "NKGGameFramework.GodotPlaneSample.PlaneGameBridge");
        var reset = RequireStaticMethodExact(bridgeType, "ResetSession");
        var step = RequireStaticMethodExact(bridgeType, "StepSession");
        var handle = RequireStaticMethodExact(bridgeType, "HandleDebugRequest", typeof(string));

        string? dumpPath = null;
        try
        {
            reset.Invoke(null, []);
            step.Invoke(null, []);

            var health = SendBridgeRequest(handle, "GET", "/_nkg/debug/health");
            Require(health.StatusCode == 200 && health.Body.Contains("\"status\":\"ok\"", StringComparison.Ordinal),
                "Diagnostics bridge health endpoint failed");

            var snapshot = SendBridgeRequest(
                handle,
                "GET",
                "/_nkg/debug/snapshot?includePayload=true&includeStructured=true&waitForFrame=false");
            Require(snapshot.StatusCode == 200, "Diagnostics bridge snapshot endpoint failed");
            var target = FindFirstMutableComponentTarget(snapshot.Body);

            var pause = SendBridgeRequest(handle, "POST", "/_nkg/debug/control", "{\"command\":\"pause\"}");
            Require(pause.StatusCode == 200 &&
                pause.Body.Contains("\"succeeded\":true", StringComparison.Ordinal) &&
                pause.Body.Contains("\"isPaused\":true", StringComparison.Ordinal),
                "Diagnostics bridge pause control failed");

            var mutationJson =
                "{\"worldName\":" + QuoteJsonString(target.WorldName) +
                ",\"sceneName\":" + QuoteJsonString(target.SceneName) +
                ",\"entityId\":" + target.EntityId.ToString(CultureInfo.InvariantCulture) +
                ",\"entityVersion\":" + target.EntityVersion.ToString(CultureInfo.InvariantCulture) +
                ",\"componentTypeFullName\":" + QuoteJsonString(target.ComponentTypeFullName) +
                ",\"componentAssemblyName\":" + QuoteJsonString(target.ComponentAssemblyName) +
                ",\"value\":" + target.ValueJson +
                "}";
            var mutation = SendBridgeRequest(handle, "POST", "/_nkg/debug/mutations", mutationJson);
            Require(mutation.StatusCode == 200 && mutation.Body.Contains("\"succeeded\":true", StringComparison.Ordinal),
                "Diagnostics bridge mutation endpoint failed");

            var play = SendBridgeRequest(handle, "POST", "/_nkg/debug/control", "{\"command\":\"play\"}");
            Require(play.StatusCode == 200 && play.Body.Contains("\"succeeded\":true", StringComparison.Ordinal),
                "Diagnostics bridge play control failed");

            var start = SendBridgeRequest(
                handle,
                "POST",
                "/_nkg/debug/dump/recording",
                "{\"command\":\"start\",\"name\":\"leanclr-webdebug-smoke\"}");
            Require(start.StatusCode == 200 &&
                start.Body.Contains("\"succeeded\":true", StringComparison.Ordinal) &&
                start.Body.Contains("\"isRecording\":true", StringComparison.Ordinal),
                "Diagnostics bridge dump recording start failed");

            step.Invoke(null, []);
            step.Invoke(null, []);

            var stop = SendBridgeRequest(handle, "POST", "/_nkg/debug/dump/recording", "{\"command\":\"stop\"}");
            Require(stop.StatusCode == 200 && stop.Body.Contains("\"succeeded\":true", StringComparison.Ordinal),
                "Diagnostics bridge dump recording stop failed");
            var stopState = RequireJsonObjectProperty(stop.Body, "state");
            dumpPath = RequireJsonStringProperty(stopState, "lastDumpPath");
            Require(!string.IsNullOrWhiteSpace(dumpPath) && File.Exists(dumpPath),
                "Diagnostics bridge dump file was not written");

            var dumpBytes = File.ReadAllBytes(dumpPath);
            var analysis = SendBridgeRequest(handle, "POST", "/_nkg/debug/dump/analysis/upload", dumpBytes);
            Require(analysis.StatusCode == 200,
                "Diagnostics bridge dump analysis endpoint failed: " +
                analysis.StatusCode.ToString(CultureInfo.InvariantCulture) + " " + analysis.Body);
            var total = RequireJsonObjectProperty(analysis.Body, "total");
            Require(RequireJsonIntProperty(analysis.Body, "frameCount") >= 2 &&
                RequireJsonIntProperty(total, "totalBytes") > 0,
                "Diagnostics bridge dump analysis did not report payload bytes");

            var playback = SendBridgeRequest(handle, "POST", "/_nkg/debug/dump/playback/upload", dumpBytes);
            Require(playback.StatusCode == 200,
                "Diagnostics bridge dump playback upload failed: " +
                playback.StatusCode.ToString(CultureInfo.InvariantCulture) + " " + playback.Body);
            var playbackId = RequireJsonStringProperty(playback.Body, "id");

            var playbackFrame = SendBridgeRequest(
                handle,
                "GET",
                "/_nkg/debug/dump/playback/frame?playbackId=" + playbackId + "&frameIndex=0");
            Require(playbackFrame.StatusCode == 200 &&
                playbackFrame.Body.Contains("\"snapshot\":", StringComparison.Ordinal) &&
                playbackFrame.Body.Contains("\"worlds\":[", StringComparison.Ordinal),
                "Diagnostics bridge dump playback frame failed: " +
                playbackFrame.StatusCode.ToString(CultureInfo.InvariantCulture) + " " + playbackFrame.Body);

            var playbackComponent = SendBridgeRequest(
                handle,
                "GET",
                "/_nkg/debug/dump/playback/component?playbackId=" + playbackId +
                "&frameIndex=0&worldName=" + target.WorldName +
                "&sceneName=" + target.SceneName +
                "&entityId=" + target.EntityId.ToString(CultureInfo.InvariantCulture) +
                "&componentTypeFullName=" + target.ComponentTypeFullName +
                "&componentAssemblyName=" + target.ComponentAssemblyName);
            Require(playbackComponent.StatusCode == 200 &&
                playbackComponent.Body.Contains(target.ComponentTypeFullName, StringComparison.Ordinal) &&
                playbackComponent.Body.Contains("\"structured\":{", StringComparison.Ordinal),
                "Diagnostics bridge dump playback component failed: " +
                playbackComponent.StatusCode.ToString(CultureInfo.InvariantCulture) + " " + playbackComponent.Body);
        }
        finally
        {
            if (!string.IsNullOrWhiteSpace(dumpPath) && File.Exists(dumpPath))
            {
                File.Delete(dumpPath);
            }
        }
    }

    public static void RunOdinSerializerPrimitiveProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var serializer = CreateOdinGameSerializer(serializerType);

        var serializeToBytes = RequireInstanceMethod(serializerType, "SerializeToBytes", parameterCount: 1)
            .MakeGenericMethod(typeof(int));
        var deserializeFromBytes = RequireInstanceMethod(serializerType, "DeserializeFromBytes", parameterCount: 1)
            .MakeGenericMethod(typeof(int));

        var payload = (byte[])serializeToBytes.Invoke(serializer, [42])!;
        Require(payload.Length > 0, "Odin primitive payload was empty");
        var restored = (int)deserializeFromBytes.Invoke(serializer, [payload])!;
        Require(restored == 42, "Odin primitive roundtrip mismatch");
    }

    public static void RunOdinSerializerConstructionProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        _ = CreateOdinGameSerializer(serializerType);
    }

    public static void RunOdinSerializerConstructionWithExplicitPolicyProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var policiesType = RequireType(odinAssembly, "OdinSerializer.SerializationPolicies");
        var policy = policiesType.GetProperty("Everything", BindingFlags.Public | BindingFlags.Static)!.GetValue(null);
        Require(policy != null, "Odin SerializationPolicies.Everything returned null");

        _ = CreateOdinGameSerializer(serializerType, policy);
    }

    public static void RunOdinSerializerUninitializedObjectProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");

        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        Require(serializer != null, "OdinGameSerializer uninitialized allocation failed");
        Require(serializer.GetType() == serializerType, "OdinGameSerializer uninitialized allocation returned wrong type");
    }

    public static void RunOdinSerializerPrivateFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var errorHandlingPolicyType = RequireType(odinAssembly, "OdinSerializer.ErrorHandlingPolicy");
        var policiesType = RequireType(odinAssembly, "OdinSerializer.SerializationPolicies");

        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        Require(serializer != null, "OdinGameSerializer uninitialized allocation failed");

        SetField(serializerType, serializer, "_format", RequireEnumValue(dataFormatType, "Binary"));
        SetField(serializerType, serializer, "_policy", policiesType.GetProperty("Everything", BindingFlags.Public | BindingFlags.Static)!.GetValue(null));
        SetField(serializerType, serializer, "_loggingPolicy", RequireEnumValue(loggingPolicyType, "Silent"));
        SetField(serializerType, serializer, "_errorHandlingPolicy", RequireEnumValue(errorHandlingPolicyType, "ThrowOnErrors"));
    }

    public static void RunOdinSerializerPrivateFormatFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_format", RequireEnumValue(dataFormatType, "Binary"));
    }

    public static void RunOdinSerializerPrivateFormatJsonFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_format", RequireEnumValue(dataFormatType, "JSON"));
    }

    public static void RunOdinSerializerPrivatePolicyFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var policiesType = RequireType(odinAssembly, "OdinSerializer.SerializationPolicies");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_policy", policiesType.GetProperty("Everything", BindingFlags.Public | BindingFlags.Static)!.GetValue(null));
    }

    public static void RunOdinSerializerPrivateLoggingFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_loggingPolicy", RequireEnumValue(loggingPolicyType, "Silent"));
    }

    public static void RunOdinSerializerPrivateLoggingZeroFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_loggingPolicy", RequireEnumValue(loggingPolicyType, "LogErrors"));
    }

    public static void RunOdinSerializerPrivateErrorHandlingFieldSetProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var errorHandlingPolicyType = RequireType(odinAssembly, "OdinSerializer.ErrorHandlingPolicy");
        var serializer = RuntimeHelpers.GetUninitializedObject(serializerType);
        SetField(serializerType, serializer, "_errorHandlingPolicy", RequireEnumValue(errorHandlingPolicyType, "ThrowOnErrors"));
    }

    public static void RunOdinSerializerConstructorMetadataProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var policyType = RequireType(odinAssembly, "OdinSerializer.ISerializationPolicy");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var errorHandlingPolicyType = RequireType(odinAssembly, "OdinSerializer.ErrorHandlingPolicy");

        var constructor = serializerType.GetConstructor(
            BindingFlags.Public | BindingFlags.Instance,
            binder: null,
            [dataFormatType, policyType, loggingPolicyType, errorHandlingPolicyType],
            modifiers: null);
        Require(constructor != null, "OdinGameSerializer default constructor surface missing");
        Require(constructor!.GetParameters().Length == 4, "OdinGameSerializer constructor parameter count mismatch");
    }

    public static void RunOdinSerializerEnumValueProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var errorHandlingPolicyType = RequireType(odinAssembly, "OdinSerializer.ErrorHandlingPolicy");

        _ = RequireEnumValue(dataFormatType, "Binary");
        _ = RequireEnumValue(loggingPolicyType, "Silent");
        _ = RequireEnumValue(errorHandlingPolicyType, "ThrowOnErrors");
    }

    public static void RunOdinSerializerEnumTypeEqualityProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");

        var json = RequireEnumValue(dataFormatType, "JSON");
        var silent = RequireEnumValue(loggingPolicyType, "Silent");
        Require(json.GetType() == dataFormatType, "DataFormat.JSON boxed enum type mismatch");
        Require(silent.GetType() == loggingPolicyType, "LoggingPolicy.Silent boxed enum type mismatch");
    }

    public static void RunOdinSerializerEnumIsInstanceProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");

        var json = RequireEnumValue(dataFormatType, "JSON");
        var silent = RequireEnumValue(loggingPolicyType, "Silent");
        Require(dataFormatType.IsInstanceOfType(json), "DataFormat.JSON IsInstanceOfType failed");
        Require(loggingPolicyType.IsInstanceOfType(silent), "LoggingPolicy.Silent IsInstanceOfType failed");
    }

    public static void RunOdinCustomSerializationPolicyConstructionProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var customPolicyType = RequireType(odinAssembly, "OdinSerializer.CustomSerializationPolicy");
        var constructor = customPolicyType.GetConstructor(
            BindingFlags.Public | BindingFlags.Instance,
            binder: null,
            [typeof(string), typeof(bool), typeof(Func<MemberInfo, bool>)],
            modifiers: null);
        Require(constructor != null, "Odin CustomSerializationPolicy constructor surface missing");

        var policy = constructor!.Invoke([
            "LeanClrProbe.Policy",
            true,
            new Func<MemberInfo, bool>(static member => member is FieldInfo),
        ]);
        Require(policy != null, "Odin CustomSerializationPolicy construction failed");
    }

    public static void RunOdinSerializationPoliciesEverythingProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var policiesType = RequireType(odinAssembly, "OdinSerializer.SerializationPolicies");
        var property = policiesType.GetProperty("Everything", BindingFlags.Public | BindingFlags.Static);
        Require(property != null, "Odin SerializationPolicies.Everything property missing");

        var policy = property!.GetValue(null);
        Require(policy != null, "Odin SerializationPolicies.Everything returned null");
    }

    public static void RunOdinSerializerPrimitiveSerializeProbe()
    {
        var nkgAssembly = Assembly.Load("NKGGameFramework");
        var serializerType = RequireType(nkgAssembly, "NKGGameFramework.Serialization.OdinGameSerializer");
        var serializer = CreateOdinGameSerializer(serializerType);

        var serializeToBytes = RequireInstanceMethod(serializerType, "SerializeToBytes", parameterCount: 1)
            .MakeGenericMethod(typeof(int));

        var payload = (byte[])serializeToBytes.Invoke(serializer, [42])!;
        Require(payload.Length > 0, "Odin primitive payload was empty");
    }

    public static void RunOdinSerializationUtilityWeakPrimitiveProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var serializationContextType = RequireType(odinAssembly, "OdinSerializer.SerializationContext");
        var serializationUtilityType = RequireType(odinAssembly, "OdinSerializer.SerializationUtility");

        var format = RequireEnumValue(dataFormatType, "Binary");
        var context = Activator.CreateInstance(serializationContextType);
        Require(context != null, "Odin SerializationContext construction failed");

        var serializeValueWeak = RequireStaticMethodExact(
            serializationUtilityType,
            "SerializeValueWeak",
            typeof(object),
            dataFormatType,
            serializationContextType);

        var payload = (byte[])serializeValueWeak.Invoke(null, [42, format, context])!;
        Require(payload.Length > 0, "Odin weak primitive payload was empty");
    }

    public static void RunOdinSerializationUtilityWeakSnapshotProbe()
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var serializationContextType = RequireType(odinAssembly, "OdinSerializer.SerializationContext");
        var serializationUtilityType = RequireType(odinAssembly, "OdinSerializer.SerializationUtility");
        var snapshot = CreateSamplerSnapshot(out _);

        var format = RequireEnumValue(dataFormatType, "Binary");
        var context = Activator.CreateInstance(serializationContextType);
        Require(context != null, "Odin SerializationContext construction failed");

        var serializeValueWeak = RequireStaticMethodExact(
            serializationUtilityType,
            "SerializeValueWeak",
            typeof(object),
            dataFormatType,
            serializationContextType);

        var payload = (byte[])serializeValueWeak.Invoke(null, [snapshot, format, context])!;
        Require(payload.Length > 0, "Odin weak snapshot payload was empty");
    }

    private static async ValueTask<int> AddValueTaskAsync(int left, int right)
    {
        var awaitedLeft = await new ValueTask<int>(left);
        return awaitedLeft + right;
    }

    private static bool Never()
    {
        return DateTime.UtcNow.Ticks < 0;
    }

    public static void RunReflectionAttributeSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework");
        Require(assembly.GetName().Name == "NKGGameFramework", "NKG assembly load failed");

        var references = assembly.GetReferencedAssemblies();
        Require(ContainsAssemblyReference(references, "OdinSerializer"), "NKG Odin reference missing");
        Require(ContainsAssemblyReference(references, "UniTask"), "NKG UniTask reference missing");

        var types = assembly.GetTypes();
        Require(types.Length > 0, "NKG type enumeration failed");

        bool sawCore = false;
        bool sawSerialization = false;
        int nkgTypeCount = 0;
        int reflectedMemberCount = 0;
        int customAttributeDataCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var type = types[i];
            var ns = type.Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            nkgTypeCount++;
            sawCore |= ns == "NKGGameFramework.Core";
            sawSerialization |= ns == "NKGGameFramework.Serialization";
            customAttributeDataCount += type.GetCustomAttributesData().Count;

            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;

            var fields = type.GetFields(flags);
            reflectedMemberCount += fields.Length;
            for (int j = 0; j < fields.Length; j++)
            {
                customAttributeDataCount += fields[j].GetCustomAttributesData().Count;
            }

            var properties = type.GetProperties(flags);
            reflectedMemberCount += properties.Length;
            for (int j = 0; j < properties.Length; j++)
            {
                customAttributeDataCount += properties[j].GetCustomAttributesData().Count;
            }

            var methods = type.GetMethods(flags);
            reflectedMemberCount += methods.Length;
            for (int j = 0; j < methods.Length; j++)
            {
                customAttributeDataCount += methods[j].GetCustomAttributesData().Count;
            }
        }

        Require(sawCore, "NKG core namespace missing");
        Require(sawSerialization, "NKG serialization namespace missing");
        Require(nkgTypeCount >= 20, "NKG type count too small");
        Require(reflectedMemberCount >= 50, "NKG reflected member count too small");
        Require(customAttributeDataCount > 0, "NKG custom attribute data enumeration failed");

        var serializerType = assembly.GetType("NKGGameFramework.Serialization.OdinGameSerializer");
        Require(serializerType != null, "NKG OdinGameSerializer type lookup failed");
        Require(serializerType!.GetConstructors(BindingFlags.Public | BindingFlags.Instance).Length > 0, "NKG OdinGameSerializer constructors missing");
    }

    public static void RunAsyncAndSerializationSurfaceSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework");

        var gameAsyncType = RequireType(assembly, "NKGGameFramework.Async.GameAsync");
        var gameTimerType = RequireType(assembly, "NKGGameFramework.Core.IGameTimer");
        RequireProperty(gameAsyncType, "CompletedTask", "Cysharp.Threading.Tasks.UniTask");
        RequireMethod(gameAsyncType, "FromResult", null, parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(gameAsyncType, "WhenAll", "Cysharp.Threading.Tasks.UniTask", parameterCount: 1, genericArgumentCount: 0);
        RequireMethod(gameAsyncType, "WhenAll", null, parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(gameAsyncType, "WhenAny", null, parameterCount: 1, genericArgumentCount: 0);
        RequireMethod(gameAsyncType, "Delay", "Cysharp.Threading.Tasks.UniTask", parameterCount: 3, genericArgumentCount: 0);
        RequireMethod(gameAsyncType, "NextFrame", "Cysharp.Threading.Tasks.UniTask", parameterCount: 2, genericArgumentCount: 0);
        RequireMethod(gameAsyncType, "DelayFrame", "Cysharp.Threading.Tasks.UniTask", parameterCount: 3, genericArgumentCount: 0);
        RequireMethod(gameTimerType, "DelayAsync", "Cysharp.Threading.Tasks.UniTask", parameterCount: 2, genericArgumentCount: 0);
        RequireMethod(gameTimerType, "NextFrameAsync", "Cysharp.Threading.Tasks.UniTask", parameterCount: 1, genericArgumentCount: 0);
        RequireMethod(gameTimerType, "DelayFrameAsync", "Cysharp.Threading.Tasks.UniTask", parameterCount: 2, genericArgumentCount: 0);

        var gameSerializerType = RequireType(assembly, "NKGGameFramework.Serialization.IGameSerializer");
        var binarySerializerType = RequireType(assembly, "NKGGameFramework.Serialization.IBinaryGameSerializer");
        var jsonSerializerType = RequireType(assembly, "NKGGameFramework.Serialization.IJsonGameSerializer");
        var odinSerializerType = RequireType(assembly, "NKGGameFramework.Serialization.OdinGameSerializer");

        RequireMethod(gameSerializerType, "Serialize", "System.String", parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(gameSerializerType, "Deserialize", null, parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(binarySerializerType, "SerializeToBytes", "System.Byte[]", parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(binarySerializerType, "DeserializeFromBytes", null, parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(jsonSerializerType, "SerializeToJson", "System.String", parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(jsonSerializerType, "DeserializeFromJson", null, parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(odinSerializerType, "Serialize", "System.String", parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(odinSerializerType, "SerializeToBytes", "System.Byte[]", parameterCount: 1, genericArgumentCount: 1);
        RequireMethod(odinSerializerType, "SerializeToJson", "System.String", parameterCount: 1, genericArgumentCount: 1);
    }

    public static void RunSamplerCoreGameplaySmoke()
    {
        RunSampleGameUpdates(updateCount: 4, expectedFrame: 2, expectSnapshot: true);
    }

    public static void RunGodotPlaneWebDebugSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework.GodotPlaneSample");
        var bridgeType = RequireType(assembly, "NKGGameFramework.GodotPlaneSample.PlaneGameBridge");

        InvokeStatic(bridgeType, "ResetSession");

        var snapshot = InvokeStatic(bridgeType, "StepSession") as string;
        Require(!string.IsNullOrWhiteSpace(snapshot), "Godot plane bridge snapshot was empty");
        Require(snapshot!.StartsWith("NKGCB1\nbase64\n", StringComparison.Ordinal), "Godot plane bridge snapshot envelope missing");

        var commandBytes = InvokeStatic(bridgeType, "StepSessionCommandBytes") as byte[];
        Require(commandBytes != null && commandBytes.Length >= 2, "Godot plane bridge command bytes were empty");
        Require(commandBytes![0] == 1 && commandBytes[^1] == 255, "Godot plane bridge command byte envelope mismatch");

        var health = InvokeStatic(bridgeType, "HandleDebugRequest", "GET\n/_nkg/debug/health\n") as string;
        Require(!string.IsNullOrWhiteSpace(health), "Godot plane debug health response was empty");
        Require(health!.StartsWith("200\nOK\n", StringComparison.Ordinal), "Godot plane debug health status mismatch");
        Require(health.Contains("\"status\":\"ok\"", StringComparison.Ordinal), "Godot plane debug health payload mismatch");

        var debugSnapshot = InvokeStatic(bridgeType, "HandleDebugRequest", "GET\n/_nkg/debug/snapshot?includePayload=false&includeStructured=false&waitForFrame=false\n") as string;
        Require(!string.IsNullOrWhiteSpace(debugSnapshot), "Godot plane debug snapshot response was empty");
        Require(debugSnapshot!.StartsWith("200\nOK\n", StringComparison.Ordinal), "Godot plane debug snapshot status mismatch");
        Require(debugSnapshot.Contains("\"snapshot\":", StringComparison.Ordinal), "Godot plane debug snapshot payload missing");
        Require(debugSnapshot.Contains("\"worlds\"", StringComparison.Ordinal), "Godot plane debug snapshot worlds missing");
        Require(debugSnapshot.Contains("godot-plane-world", StringComparison.Ordinal), "Godot plane debug snapshot world name missing");
    }

    public static void RunHostingWebDebugStartSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework.Hosting");
        var hostType = RequireType(assembly, "NKGGameFramework.Hosting.Diagnostics.GameDebugHost");

        var startMethod = RequireStaticMethod(hostType, "StartAsync", parameterCount: 1);
        var startTask = (Task)startMethod.Invoke(null, [default(CancellationToken)])!;
        startTask.GetAwaiter().GetResult();

        var host = RequirePropertyValue(startTask.GetType(), startTask, "Result");
        var baseAddress = RequirePropertyValue(hostType, host, "BaseAddress");
        Require(baseAddress.ToString()!.StartsWith("http://127.0.0.1:", StringComparison.Ordinal), "NKG Hosting debug host base address mismatch");

        var disposeAsync = RequireInstanceMethod(hostType, "DisposeAsync", parameterCount: 0);
        var disposeTask = (ValueTask)disposeAsync.Invoke(host, [])!;
        disposeTask.GetAwaiter().GetResult();
    }

    public static void RunSamplerOdinRoundTripProbe()
    {
        RunSampleGameUpdates(updateCount: 5, expectedFrame: 3, expectSnapshot: false);
    }

    private static void RunSampleGameUpdates(int updateCount, int expectedFrame, bool expectSnapshot)
    {
        var assembly = Assembly.Load("NKGGameFramework.Sampler");
        var sampleGameType = RequireType(assembly, "NKGGameFramework.Sampler.SampleGame");
        var game = Activator.CreateInstance(sampleGameType, nonPublic: true);
        Require(game != null, "NKG SampleGame construction failed");

        InvokeInstance(sampleGameType, game!, "Start");
        Require((bool)RequirePropertyValue(sampleGameType, game!, "IsRunning"), "NKG SampleGame did not start");

        for (int i = 0; i < updateCount; i++)
        {
            InvokeInstance(sampleGameType, game!, "Update", 0.016d, 0.016d);
        }

        Require((int)RequirePropertyValue(sampleGameType, game!, "Frame") == expectedFrame, "NKG SampleGame frame progression failed");
        if (expectSnapshot)
        {
            var snapshot = InvokeInstance(sampleGameType, game!, "CreateSnapshot");
            Require(snapshot != null, "NKG SampleGame snapshot creation failed");
            var snapshotType = snapshot!.GetType();
            Require((int)RequirePropertyValue(snapshotType, snapshot, "Frame") == expectedFrame, "NKG SampleGame snapshot frame mismatch");
            Require((double)RequirePropertyValue(snapshotType, snapshot, "PositionX") > 0d, "NKG ECS movement did not update X position");
            Require((double)RequirePropertyValue(snapshotType, snapshot, "PositionY") > 0d, "NKG ECS movement did not update Y position");
            Require((int)RequirePropertyValue(snapshotType, snapshot, "Health") == 8, "NKG ECS damage system did not update health");
        }

        InvokeInstance(sampleGameType, game!, "Dispose");
    }

    public static void RunAssemblyNameSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework");
        Require(assembly.GetName().Name == "NKGGameFramework", "NKG assembly load failed");
    }

    public static void RunReferencedAssembliesSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework");
        var references = assembly.GetReferencedAssemblies();
        Require(ContainsAssemblyReference(references, "OdinSerializer"), "NKG Odin reference missing");
        Require(ContainsAssemblyReference(references, "UniTask"), "NKG UniTask reference missing");
    }

    public static void RunTypesSmoke()
    {
        var assembly = Assembly.Load("NKGGameFramework");
        var types = assembly.GetTypes();
        Require(types.Length > 0, "NKG type enumeration failed");
    }

    public static void RunTypeNamespaceSmoke()
    {
        var types = LoadNkgTypes();
        int nkgTypeCount = 0;
        bool sawCore = false;
        bool sawSerialization = false;

        for (int i = 0; i < types.Length; i++)
        {
            var ns = types[i].Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            nkgTypeCount++;
            sawCore |= ns == "NKGGameFramework.Core";
            sawSerialization |= ns == "NKGGameFramework.Serialization";
        }

        Require(sawCore, "NKG core namespace missing");
        Require(sawSerialization, "NKG serialization namespace missing");
        Require(nkgTypeCount >= 20, "NKG type count too small");
    }

    public static void RunTypeCustomAttributeDataSmoke()
    {
        var types = LoadNkgTypes();
        int customAttributeDataCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var ns = types[i].Namespace ?? "";
            if (ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                var attrs = types[i].GetCustomAttributesData();
                customAttributeDataCount += attrs.Count;
            }
        }

        Require(customAttributeDataCount > 0, "NKG type custom attribute data enumeration failed");
    }

    public static void RunMemberEnumerationSmoke()
    {
        var types = LoadNkgTypes();
        int reflectedMemberCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var ns = types[i].Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;
            reflectedMemberCount += types[i].GetFields(flags).Length;
            reflectedMemberCount += types[i].GetProperties(flags).Length;
            reflectedMemberCount += types[i].GetMethods(flags).Length;
        }

        Require(reflectedMemberCount >= 50, "NKG reflected member count too small");
    }

    public static void RunMemberCustomAttributeDataSmoke()
    {
        int customAttributeDataCount =
            CountFieldCustomAttributeData() +
            CountPropertyCustomAttributeData() +
            CountMethodCustomAttributeData();

        Require(customAttributeDataCount > 0, "NKG member custom attribute data enumeration failed");
    }

    public static void RunFieldCustomAttributeDataSmoke()
    {
        _ = CountFieldCustomAttributeData();
    }

    public static void RunPropertyCustomAttributeDataSmoke()
    {
        _ = CountPropertyCustomAttributeData();
    }

    public static void RunMethodCustomAttributeDataSmoke()
    {
        _ = CountMethodCustomAttributeData();
    }

    private static int CountFieldCustomAttributeData()
    {
        var types = LoadNkgTypes();
        int customAttributeDataCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var type = types[i];
            var ns = type.Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;

            var fields = type.GetFields(flags);
            for (int j = 0; j < fields.Length; j++)
            {
                customAttributeDataCount += fields[j].GetCustomAttributesData().Count;
            }
        }

        return customAttributeDataCount;
    }

    private static int CountPropertyCustomAttributeData()
    {
        var types = LoadNkgTypes();
        int customAttributeDataCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var type = types[i];
            var ns = type.Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;

            var properties = type.GetProperties(flags);
            for (int j = 0; j < properties.Length; j++)
            {
                customAttributeDataCount += properties[j].GetCustomAttributesData().Count;
            }
        }

        return customAttributeDataCount;
    }

    private static int CountMethodCustomAttributeData()
    {
        var types = LoadNkgTypes();
        int customAttributeDataCount = 0;

        for (int i = 0; i < types.Length; i++)
        {
            var type = types[i];
            var ns = type.Namespace ?? "";
            if (!ns.StartsWith("NKGGameFramework", StringComparison.Ordinal))
            {
                continue;
            }

            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;

            var methods = type.GetMethods(flags);
            for (int j = 0; j < methods.Length; j++)
            {
                customAttributeDataCount += methods[j].GetCustomAttributesData().Count;
            }
        }

        return customAttributeDataCount;
    }

    private static Type[] LoadNkgTypes()
    {
        return Assembly.Load("NKGGameFramework").GetTypes();
    }

    private static bool ContainsAssemblyReference(AssemblyName[] references, string name)
    {
        for (int i = 0; i < references.Length; i++)
        {
            if (references[i].Name == name)
            {
                return true;
            }
        }

        return false;
    }

    private static BridgeResponse SendBridgeRequest(MethodInfo handle, string method, string target, string body)
    {
        return SendBridgeRequest(handle, method, target, Encoding.UTF8.GetBytes(body));
    }

    private static BridgeResponse SendBridgeRequest(MethodInfo handle, string method, string target)
    {
        return SendBridgeRequest(handle, method, target, []);
    }

    private static BridgeResponse SendBridgeRequest(MethodInfo handle, string method, string target, byte[] body)
    {
        var request = method + "\n" + target + "\nbase64\n" + Convert.ToBase64String(body);
        var value = handle.Invoke(null, [request]) as string;
        Require(!string.IsNullOrEmpty(value), "Diagnostics bridge returned an empty response");
        return ParseBridgeResponse(value!);
    }

    private static BridgeResponse ParseBridgeResponse(string value)
    {
        var first = value.IndexOf('\n', StringComparison.Ordinal);
        var second = first < 0 ? -1 : value.IndexOf('\n', first + 1);
        var third = second < 0 ? -1 : value.IndexOf('\n', second + 1);
        Require(first >= 0 && second >= 0 && third >= 0, "Diagnostics bridge response was malformed");
        return new BridgeResponse(
            int.Parse(value[..first], CultureInfo.InvariantCulture),
            value[(first + 1)..second],
            value[(second + 1)..third],
            value[(third + 1)..]);
    }

    private static DebugComponentTarget FindFirstMutableComponentTarget(string bridgeBody)
    {
        var snapshotMessage = RequireJsonObjectProperty(bridgeBody, "snapshot");
        var worlds = RequireJsonArrayProperty(snapshotMessage, "worlds");
        var world = RequireFirstObjectInArray(worlds, "world");
        var worldName = RequireJsonStringProperty(world, "name");
        var scenes = RequireJsonArrayProperty(world, "scenes");
        var scene = RequireFirstObjectInArray(scenes, "scene");
        var sceneName = RequireJsonStringProperty(scene, "name");
        var entities = RequireJsonArrayProperty(scene, "entities");
        var entity = RequireFirstObjectInArray(entities, "entity");
        var entityId = RequireJsonIntProperty(entity, "id");
        var entityVersion = RequireJsonIntProperty(entity, "version");
        var components = RequireJsonArrayProperty(entity, "components");
        var component = RequireFirstObjectInArrayContaining(components, "\"payload\":\"", "component payload");
        var type = RequireJsonObjectProperty(component, "type");
        var value = RequireJsonObjectProperty(component, "value");

        Require(value.Contains("\"structured\":{", StringComparison.Ordinal),
            "Diagnostics snapshot did not include a structured Odin component value");

        return new DebugComponentTarget(
            worldName,
            sceneName,
            entityId,
            entityVersion,
            RequireJsonStringProperty(type, "fullName"),
            RequireJsonStringProperty(type, "assemblyName"),
            value);
    }

    private static string RequireJsonObjectProperty(string json, string propertyName)
    {
        var start = RequireJsonPropertyValueStart(json, propertyName);
        Require(start < json.Length && json[start] == '{', propertyName + " JSON object missing");
        var end = FindBalancedEnd(json, start, '{', '}');
        return json[start..(end + 1)];
    }

    private static string RequireJsonArrayProperty(string json, string propertyName)
    {
        var start = RequireJsonPropertyValueStart(json, propertyName);
        Require(start < json.Length && json[start] == '[', propertyName + " JSON array missing");
        var end = FindBalancedEnd(json, start, '[', ']');
        return json[start..(end + 1)];
    }

    private static string RequireJsonStringProperty(string json, string propertyName)
    {
        var start = RequireJsonPropertyValueStart(json, propertyName);
        Require(start < json.Length && json[start] == '"', propertyName + " JSON string missing");
        var builder = new StringBuilder();
        for (int i = start + 1; i < json.Length; i++)
        {
            var ch = json[i];
            if (ch == '"')
            {
                return builder.ToString();
            }

            if (ch == '\\' && i + 1 < json.Length)
            {
                i++;
                var escaped = json[i];
                if (escaped == 'u' && i + 4 < json.Length)
                {
                    var hex = json.Substring(i + 1, 4);
                    if (int.TryParse(hex, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out var code))
                    {
                        builder.Append((char)code);
                        i += 4;
                        continue;
                    }
                }

                builder.Append(escaped switch
                {
                    '"' => '"',
                    '\\' => '\\',
                    '/' => '/',
                    'b' => '\b',
                    'f' => '\f',
                    'n' => '\n',
                    'r' => '\r',
                    't' => '\t',
                    _ => escaped,
                });
                continue;
            }

            builder.Append(ch);
        }

        throw new InvalidOperationException(propertyName + " JSON string was not terminated");
    }

    private static int RequireJsonIntProperty(string json, string propertyName)
    {
        var start = RequireJsonPropertyValueStart(json, propertyName);
        var end = start;
        while (end < json.Length && (char.IsDigit(json[end]) || json[end] == '-'))
        {
            end++;
        }

        Require(end > start, propertyName + " JSON integer missing");
        return int.Parse(json[start..end], CultureInfo.InvariantCulture);
    }

    private static int RequireJsonPropertyValueStart(string json, string propertyName)
    {
        var start = FindJsonPropertyValueStart(json, propertyName);
        Require(start >= 0, propertyName + " JSON property missing");
        return start;
    }

    private static int FindJsonPropertyValueStart(string json, string propertyName)
    {
        var objectDepth = 0;
        var arrayDepth = 0;
        var inString = false;
        var escaped = false;
        var stringStart = -1;

        for (int i = 0; i < json.Length; i++)
        {
            var ch = json[i];
            if (inString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (ch == '\\')
                {
                    escaped = true;
                }
                else if (ch == '"')
                {
                    inString = false;
                    if (objectDepth == 1 && arrayDepth == 0 && stringStart >= 0)
                    {
                        var candidate = json[(stringStart + 1)..i];
                        if (StringComparer.Ordinal.Equals(candidate, propertyName))
                        {
                            var cursor = i + 1;
                            while (cursor < json.Length && char.IsWhiteSpace(json[cursor]))
                            {
                                cursor++;
                            }

                            if (cursor < json.Length && json[cursor] == ':')
                            {
                                cursor++;
                                while (cursor < json.Length && char.IsWhiteSpace(json[cursor]))
                                {
                                    cursor++;
                                }

                                return cursor;
                            }
                        }
                    }

                    stringStart = -1;
                }

                continue;
            }

            if (ch == '"')
            {
                inString = true;
                stringStart = i;
            }
            else if (ch == '{')
            {
                objectDepth++;
            }
            else if (ch == '}')
            {
                objectDepth--;
            }
            else if (ch == '[')
            {
                arrayDepth++;
            }
            else if (ch == ']')
            {
                arrayDepth--;
            }
        }

        return -1;
    }

    private static string RequireFirstObjectInArray(string jsonArray, string label)
    {
        return RequireFirstObjectInArrayContaining(jsonArray, null, label);
    }

    private static string RequireFirstObjectInArrayContaining(string jsonArray, string? marker, string label)
    {
        var cursor = 0;
        while (cursor < jsonArray.Length)
        {
            var start = jsonArray.IndexOf('{', cursor);
            if (start < 0)
            {
                break;
            }

            var end = FindBalancedEnd(jsonArray, start, '{', '}');
            var candidate = jsonArray[start..(end + 1)];
            if (marker is null || candidate.Contains(marker, StringComparison.Ordinal))
            {
                return candidate;
            }

            cursor = end + 1;
        }

        throw new InvalidOperationException("Diagnostics JSON did not contain " + label);
    }

    private static int FindBalancedEnd(string text, int start, char open, char close)
    {
        var depth = 0;
        var inString = false;
        var escaped = false;
        for (int i = start; i < text.Length; i++)
        {
            var ch = text[i];
            if (inString)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (ch == '\\')
                {
                    escaped = true;
                }
                else if (ch == '"')
                {
                    inString = false;
                }

                continue;
            }

            if (ch == '"')
            {
                inString = true;
            }
            else if (ch == open)
            {
                depth++;
            }
            else if (ch == close)
            {
                depth--;
                if (depth == 0)
                {
                    return i;
                }
            }
        }

        throw new InvalidOperationException("Diagnostics JSON value was not balanced");
    }

    private static string QuoteJsonString(string value)
    {
        var builder = new StringBuilder();
        builder.Append('"');
        for (int i = 0; i < value.Length; i++)
        {
            var ch = value[i];
            switch (ch)
            {
                case '"':
                    builder.Append("\\\"");
                    break;
                case '\\':
                    builder.Append("\\\\");
                    break;
                case '\b':
                    builder.Append("\\b");
                    break;
                case '\f':
                    builder.Append("\\f");
                    break;
                case '\n':
                    builder.Append("\\n");
                    break;
                case '\r':
                    builder.Append("\\r");
                    break;
                case '\t':
                    builder.Append("\\t");
                    break;
                default:
                    if (ch < ' ')
                    {
                        builder.Append("\\u").Append(((int)ch).ToString("x4", CultureInfo.InvariantCulture));
                    }
                    else
                    {
                        builder.Append(ch);
                    }
                    break;
            }
        }
        builder.Append('"');
        return builder.ToString();
    }

    private static Type RequireType(Assembly assembly, string fullName)
    {
        var type = assembly.GetType(fullName);
        Require(type != null, fullName + " type lookup failed");
        return type!;
    }

    private static MethodInfo RequireStaticMethod(Type declaringType, string name, int parameterCount)
    {
        return RequireMethodInfo(declaringType, name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static, parameterCount);
    }

    private static MethodInfo RequireInstanceMethod(Type declaringType, string name, int parameterCount)
    {
        return RequireMethodInfo(declaringType, name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance, parameterCount);
    }

    private static MethodInfo RequireMethodInfo(Type declaringType, string name, BindingFlags flags, int parameterCount)
    {
        var methods = declaringType.GetMethods(flags);
        for (int i = 0; i < methods.Length; i++)
        {
            var method = methods[i];
            if (method.Name == name && method.GetParameters().Length == parameterCount)
            {
                return method;
            }
        }

        throw new MissingMethodException(declaringType.FullName, name);
    }

    private static object? InvokeInstance(Type declaringType, object instance, string name, params object[] arguments)
    {
        var methods = declaringType.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
        for (int i = 0; i < methods.Length; i++)
        {
            var method = methods[i];
            if (method.Name == name && method.GetParameters().Length == arguments.Length)
            {
                return method.Invoke(instance, arguments);
            }
        }

        throw new MissingMethodException(declaringType.FullName, name);
    }

    private static object? InvokeStatic(Type declaringType, string name, params object[] arguments)
    {
        var methods = declaringType.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
        for (int i = 0; i < methods.Length; i++)
        {
            var method = methods[i];
            if (method.Name == name && method.GetParameters().Length == arguments.Length)
            {
                return method.Invoke(null, arguments);
            }
        }

        throw new MissingMethodException(declaringType.FullName, name);
    }

    private static object RequirePropertyValue(Type declaringType, object instance, string name)
    {
        var property = declaringType.GetProperty(name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
        Require(property != null, declaringType.FullName + "." + name + " property missing");
        var value = property!.GetValue(instance);
        Require(value != null, declaringType.FullName + "." + name + " property returned null");
        return value!;
    }

    private static object CreateSamplerSnapshot(out Type snapshotType)
    {
        var samplerAssembly = Assembly.Load("NKGGameFramework.Sampler");
        snapshotType = RequireType(samplerAssembly, "NKGGameFramework.Sampler.GameSnapshot");
        var snapshot = Activator.CreateInstance(snapshotType, nonPublic: true);
        Require(snapshot != null, "GameSnapshot construction failed");
        SetProperty(snapshotType, snapshot!, "Frame", 9);
        SetProperty(snapshotType, snapshot!, "PlayerName", "leanclr-odin");
        SetProperty(snapshotType, snapshot!, "PositionX", 12.5d);
        SetProperty(snapshotType, snapshot!, "PositionY", 7.25d);
        SetProperty(snapshotType, snapshot!, "Health", 6);
        return snapshot!;
    }

    private static object CreateOdinGameSerializer(Type serializerType, object? policy = null)
    {
        var odinAssembly = Assembly.Load("OdinSerializer");
        var dataFormatType = RequireType(odinAssembly, "OdinSerializer.DataFormat");
        var policyType = RequireType(odinAssembly, "OdinSerializer.ISerializationPolicy");
        var loggingPolicyType = RequireType(odinAssembly, "OdinSerializer.LoggingPolicy");
        var errorHandlingPolicyType = RequireType(odinAssembly, "OdinSerializer.ErrorHandlingPolicy");

        var constructor = serializerType.GetConstructor(
            BindingFlags.Public | BindingFlags.Instance,
            binder: null,
            [dataFormatType, policyType, loggingPolicyType, errorHandlingPolicyType],
            modifiers: null);
        Require(constructor != null, "OdinGameSerializer default constructor surface missing");

        var serializer = constructor!.Invoke([
            RequireEnumValue(dataFormatType, "Binary"),
            policy,
            RequireEnumValue(loggingPolicyType, "Silent"),
            RequireEnumValue(errorHandlingPolicyType, "ThrowOnErrors"),
        ]);
        Require(serializer != null, "OdinGameSerializer construction failed");
        return serializer!;
    }

    private static void SetProperty(Type declaringType, object instance, string name, object value)
    {
        var property = declaringType.GetProperty(name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
        Require(property != null, declaringType.FullName + "." + name + " property missing");
        property!.SetValue(instance, value);
    }

    private static void SetField(Type declaringType, object instance, string name, object? value)
    {
        var field = declaringType.GetField(name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
        Require(field != null, declaringType.FullName + "." + name + " field missing");
        field!.SetValue(instance, value);
    }

    private static object RequireEnumValue(Type enumType, string name)
    {
        Require(enumType.IsEnum, enumType.FullName + " is not an enum");
        return Enum.Parse(enumType, name);
    }

    private static MethodInfo RequireStaticMethodExact(Type declaringType, string name, params Type[] parameterTypes)
    {
        var methods = declaringType.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
        for (int i = 0; i < methods.Length; i++)
        {
            var method = methods[i];
            if (method.Name != name || method.GetParameters().Length != parameterTypes.Length)
            {
                continue;
            }

            var parameters = method.GetParameters();
            bool matched = true;
            for (int j = 0; j < parameters.Length; j++)
            {
                if (parameters[j].ParameterType != parameterTypes[j])
                {
                    matched = false;
                    break;
                }
            }

            if (matched)
            {
                return method;
            }
        }

        throw new MissingMethodException(declaringType.FullName, name);
    }

    private static void AssertSnapshotRoundTrip(Type snapshotType, object? restored)
    {
        Require(restored != null, "Odin restored snapshot was null");
        Require((int)RequirePropertyValue(snapshotType, restored!, "Frame") == 9, "Odin restored frame mismatch");
        Require((string)RequirePropertyValue(snapshotType, restored!, "PlayerName") == "leanclr-odin", "Odin restored player mismatch");
        Require((double)RequirePropertyValue(snapshotType, restored!, "PositionX") == 12.5d, "Odin restored X mismatch");
        Require((double)RequirePropertyValue(snapshotType, restored!, "PositionY") == 7.25d, "Odin restored Y mismatch");
        Require((int)RequirePropertyValue(snapshotType, restored!, "Health") == 6, "Odin restored health mismatch");
    }

    private static void RequireProperty(Type declaringType, string name, string expectedPropertyTypeName)
    {
        var property = declaringType.GetProperty(name, BindingFlags.Public | BindingFlags.Static);
        Require(property != null, declaringType.FullName + "." + name + " property missing");
        Require(TypeShapeName(property!.PropertyType) == expectedPropertyTypeName, declaringType.FullName + "." + name + " property type mismatch");
    }

    private static void RequireMethod(
        Type declaringType,
        string name,
        string? expectedReturnTypeName,
        int parameterCount,
        int genericArgumentCount)
    {
        var methods = declaringType.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly);
        for (int i = 0; i < methods.Length; i++)
        {
            var method = methods[i];
            if (method.Name != name ||
                method.GetParameters().Length != parameterCount ||
                method.GetGenericArguments().Length != genericArgumentCount)
            {
                continue;
            }

            if (expectedReturnTypeName == null || TypeShapeName(method.ReturnType) == expectedReturnTypeName)
            {
                return;
            }
        }

        throw new InvalidOperationException(declaringType.FullName + "." + name + " method surface missing");
    }

    private static string? TypeShapeName(Type type)
    {
        if (type.IsGenericType)
        {
            return type.GetGenericTypeDefinition().FullName;
        }

        return type.FullName;
    }

    private static void Require([DoesNotReturnIf(false)] bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }

    private readonly struct BridgeResponse(
        int statusCode,
        string reasonPhrase,
        string contentType,
        string body)
    {
        public int StatusCode { get; } = statusCode;

        public string ReasonPhrase { get; } = reasonPhrase;

        public string ContentType { get; } = contentType;

        public string Body { get; } = body;
    }

    private readonly struct DebugComponentTarget(
        string worldName,
        string sceneName,
        int entityId,
        int entityVersion,
        string componentTypeFullName,
        string componentAssemblyName,
        string valueJson)
    {
        public string WorldName { get; } = worldName;

        public string SceneName { get; } = sceneName;

        public int EntityId { get; } = entityId;

        public int EntityVersion { get; } = entityVersion;

        public string ComponentTypeFullName { get; } = componentTypeFullName;

        public string ComponentAssemblyName { get; } = componentAssemblyName;

        public string ValueJson { get; } = valueJson;
    }

    private readonly struct LinqRow(string group, int score)
    {
        public string Group { get; } = group;

        public int Score { get; } = score;
    }

    private readonly struct LinqBucket(string key, int[] scores, int total)
    {
        public string Key { get; } = key;

        public int[] Scores { get; } = scores;

        public int Total { get; } = total;
    }

    private enum LocalDataFormat
    {
        Binary = 1,
        Json = 2,
    }

    private enum LocalLoggingPolicy
    {
        Silent = 1,
        Verbose = 2,
    }

    private enum LocalErrorHandlingPolicy
    {
        ThrowOnErrors = 1,
        Ignore = 2,
    }

    private interface ILocalSerializationPolicy
    {
    }

    private sealed class ReflectionEnumNullConstructorTarget(
        LocalDataFormat format,
        ILocalSerializationPolicy? policy,
        LocalLoggingPolicy logging,
        LocalErrorHandlingPolicy errorHandling)
    {
        public LocalDataFormat Format { get; } = format;

        public ILocalSerializationPolicy? Policy { get; } = policy;

        public LocalLoggingPolicy Logging { get; } = logging;

        public LocalErrorHandlingPolicy ErrorHandling { get; } = errorHandling;
    }
}
