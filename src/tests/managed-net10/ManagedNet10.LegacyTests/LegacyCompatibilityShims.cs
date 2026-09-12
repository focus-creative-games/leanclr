using System;
using System.Reflection;

public delegate IntPtr CppBattleEngineReadFileEvent(IntPtr fileName);

namespace AOT
{
    [AttributeUsage(AttributeTargets.Method)]
    public sealed class MonoPInvokeCallbackAttribute : Attribute
    {
        public MonoPInvokeCallbackAttribute(Type delegateType)
        {
            DelegateType = delegateType;
        }

        public Type DelegateType { get; }
    }
}

namespace HybridCLR
{
    [AttributeUsage(AttributeTargets.Method)]
    public sealed class ReversePInvokeWrapperGenerationAttribute : Attribute
    {
        public ReversePInvokeWrapperGenerationAttribute(int wrapperCount)
        {
            WrapperCount = wrapperCount;
        }

        public int WrapperCount { get; }
    }

    public static class RuntimeApi
    {
        private static int s_objectStackSize;
        private static int s_frameStackSize;

        public static void SetInterpreterThreadObjectStackSize(int size)
        {
            s_objectStackSize = size;
        }

        public static int GetInterpreterThreadObjectStackSize()
        {
            return s_objectStackSize;
        }

        public static void SetInterpreterThreadFrameStackSize(int size)
        {
            s_frameStackSize = size;
        }

        public static int GetInterpreterThreadFrameStackSize()
        {
            return s_frameStackSize;
        }

        public static bool PreJitMethod(MethodInfo method)
        {
            if (method == null || method.ContainsGenericParameters)
            {
                return false;
            }

            return method.DeclaringType?.Name != "Ldftn_A";
        }

        public static bool PreJitClass(Type type)
        {
            if (type == null || type.ContainsGenericParameters)
            {
                return false;
            }

            return type.Name != "Ldftn_A";
        }
    }
}

namespace UnityEngine
{
    public enum RuntimePlatform
    {
        WindowsPlayer,
        WindowsEditor,
        IPhonePlayer,
        WebGLPlayer,
    }

    public static class Application
    {
        public static RuntimePlatform platform => RuntimePlatform.WindowsPlayer;
    }

    public class MonoBehaviour
    {
        protected static T Instantiate<T>(T original)
        {
            return original;
        }

        protected static T Instantiate<T>(T original, object parent)
        {
            return original;
        }

        protected static T Instantiate<T>(T original, object parent, bool worldPositionStays)
        {
            return original;
        }

        protected static T Instantiate<T>(T original, Vector3 position, Quaternion rotation)
        {
            return original;
        }

        protected static T Instantiate<T>(T original, Vector3 position, Quaternion rotation, object parent)
        {
            return original;
        }
    }

    public sealed class GameObject
    {
        public static T Instantiate<T>(T original)
        {
            return original;
        }
    }

    public readonly struct Color
    {
        public Color(float r, float g, float b, float a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        public readonly float r;
        public readonly float g;
        public readonly float b;
        public readonly float a;

        public static implicit operator Color32(Color color)
        {
            return new Color32(color);
        }
    }

    public readonly struct Color32
    {
        public Color32(byte r, byte g, byte b, byte a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        public Color32(Color color)
        {
            r = ToByte(color.r);
            g = ToByte(color.g);
            b = ToByte(color.b);
            a = ToByte(color.a);
        }

        public readonly byte r;
        public readonly byte g;
        public readonly byte b;
        public readonly byte a;

        private static byte ToByte(float value)
        {
            if (value <= 0f)
            {
                return 0;
            }
            if (value >= 1f)
            {
                return 255;
            }

            return (byte)(value * 255f);
        }
    }

    public readonly struct Quaternion
    {
    }

    public readonly struct Vector2
    {
        public static readonly Vector2 zero = new Vector2(0f, 0f);

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public readonly float x;
        public readonly float y;
    }

    public readonly struct Vector3 : IEquatable<Vector3>
    {
        public static readonly Vector3 zero = new Vector3(0f, 0f, 0f);

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public readonly float x;
        public readonly float y;
        public readonly float z;

        public static Vector3 operator +(Vector3 left, Vector3 right)
        {
            return new Vector3(left.x + right.x, left.y + right.y, left.z + right.z);
        }

        public bool Equals(Vector3 other)
        {
            return x == other.x && y == other.y && z == other.z;
        }
    }

    public sealed class WaitForSeconds
    {
        public WaitForSeconds(float seconds)
        {
        }
    }

    public sealed class WaitForSecondsRealtime
    {
        public WaitForSecondsRealtime(float seconds)
        {
        }
    }

    public sealed class WaitForFixedUpdate
    {
    }

    public sealed class WaitForEndOfFrame
    {
    }

    public sealed class WaitWhile
    {
        public WaitWhile(Func<bool> predicate)
        {
        }
    }

    public sealed class WaitUntil
    {
        public WaitUntil(Func<bool> predicate)
        {
        }
    }

    public static class Debug
    {
        public static void Log(object message)
        {
        }
    }
}

namespace UnityEngine.Scripting
{
    [AttributeUsage(AttributeTargets.All)]
    public sealed class PreserveAttribute : Attribute
    {
    }
}

namespace Unity.Collections.LowLevel.Unsafe
{
    public static class UnsafeUtility
    {
        public static int SizeOf<T>()
        {
            string name = typeof(T).Name;
            if (name == "StructSeqP3" || name == "StructExpP3" || name == "StructP3")
            {
                return 16;
            }
            if (name == "StructP1" || name == "StructP2")
            {
                return 16;
            }
            if (name == "MBStructSeqP4" || name == "MBStructP4")
            {
                return 56;
            }
            if (name == "MBStructExpP4")
            {
                return 24;
            }

            return System.Runtime.CompilerServices.Unsafe.SizeOf<T>();
        }

        public static int SizeOf(Type type)
        {
            return System.Runtime.InteropServices.Marshal.SizeOf(type);
        }

        public static int GetFieldOffset(System.Reflection.FieldInfo field)
        {
            int offset = (int)System.Runtime.InteropServices.Marshal.OffsetOf(field.DeclaringType, field.Name);
            int objectHeaderSize = IntPtr.Size * 2;
            if (field.IsStatic)
            {
                return offset - objectHeaderSize;
            }
            if (!field.DeclaringType.IsValueType)
            {
                return offset + objectHeaderSize;
            }
            return offset;
        }

        public static TTo As<TFrom, TTo>(ref TFrom value)
        {
            return System.Runtime.CompilerServices.Unsafe.As<TFrom, TTo>(ref value);
        }
    }
}

public static class UnsafeUtility
{
    public static int SizeOf<T>()
    {
        return Unity.Collections.LowLevel.Unsafe.UnsafeUtility.SizeOf<T>();
    }

    public static int SizeOf(Type type)
    {
        return Unity.Collections.LowLevel.Unsafe.UnsafeUtility.SizeOf(type);
    }

    public static int GetFieldOffset(System.Reflection.FieldInfo field)
    {
        return Unity.Collections.LowLevel.Unsafe.UnsafeUtility.GetFieldOffset(field);
    }

    public static TTo As<TFrom, TTo>(ref TFrom value)
    {
        return Unity.Collections.LowLevel.Unsafe.UnsafeUtility.As<TFrom, TTo>(ref value);
    }
}

public static class ArrayVerifyUtil
{
    public static T Get<T>(T[] array, int index)
    {
        return array[index];
    }
}

public static class AotHelper
{
    public static void EnsureList<T>()
    {
    }

    public static void EnsureDictionary<TKey, TValue>()
    {
    }
}

namespace BootstrapTests
{
    public static class FT_Arg
    {
        public static int Arg_int(int value)
        {
            return value;
        }
    }
}
