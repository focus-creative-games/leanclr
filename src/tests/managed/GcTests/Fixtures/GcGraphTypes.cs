using System;

namespace GcTests.Fixtures
{
    internal class GcNode
    {
        public object data;
        public GcNode next;
    }

    internal struct RefLeaf
    {
        public object payload;
        public int tag;
    }

    internal struct NestedRefs
    {
        public RefLeaf leaf;
        public int id;
    }

    internal struct DeepNestedRefs
    {
        public NestedRefs outer;
        public long padding;
    }

    internal class RefContainer
    {
        public object field0;
        public object field1;
        public NestedRefs nested;
        public string text;
        public object[] arrayField;
    }

    internal class GcBaseWithRef
    {
        public object baseField;
    }

    internal class GcDerivedWithRef : GcBaseWithRef
    {
        public object derivedField;
    }

    /// Non-sealed base with no reference fields; derived type adds a reference field.
    internal class GcNoRefOpenBase
    {
        public int marker;
    }

    internal class GcNoRefOpenDerived : GcNoRefOpenBase
    {
        public object payload;
    }

    /// Non-sealed interface with no reference fields on the interface itself.
    internal interface IGcOpenMarker
    {
        int Marker { get; }
    }

    internal class GcOpenMarkerImpl : IGcOpenMarker
    {
        public int Marker { get; set; }
        public object payload;
    }

    internal struct PrimitiveOnlyStruct
    {
        public int a;
        public long b;
        public double c;
    }

    internal class PrimitiveOnlyContainer
    {
        public int a;
        public long b;
        public double c;
    }

    internal class SelfReferencingNode
    {
        public object payload;
        public SelfReferencingNode self;
    }

    internal class DeepNestedHolder
    {
        public DeepNestedRefs deep;
    }

    internal static class GcStaticRoots
    {
        public static object root;
        public static string staticString;
        public static NestedRefs staticNested;
        public static DeepNestedRefs staticDeepNested;
        public static RefContainer staticContainer;
        public static object[] staticArray;
    }

    /// <summary>
    /// Types used to exercise static_gc_bitmap bit index 0 (max_bitmap_index == 0).
    /// Each holder declares a single static field at layout offset 0 so finalize_gc_bitmap
    /// must not treat the bitmap as empty when scratch word 0 has a set bit.
    /// </summary>
    internal static class GcStaticBitmapBitZeroRefHolder
    {
        public static object onlyReference;
    }

    internal struct StaticValueWithRefAtLayoutZero
    {
        public object payload;
    }

    internal static class GcStaticBitmapBitZeroValueHolder
    {
        public static StaticValueWithRefAtLayoutZero valueAtZero;
    }
}
