using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Scan
{
    internal class TC_GC_ValueTypes : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Deep_nested_struct_reference_is_scanned()
        {
            var holder = new DeepNestedHolder();
            object deepChild = new object();
            holder.deep.outer.leaf.payload = deepChild;
            GCHandle holderPin = KeepAlive(holder);
            GCHandle weak = TrackWeak(deepChild);
            try
            {
                deepChild = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref holderPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_deep_nested_struct_reference_is_scanned()
        {
            object deepChild = new object();
            GcStaticRoots.staticDeepNested.outer.leaf.payload = deepChild;
            GCHandle weak = TrackWeak(deepChild);
            try
            {
                deepChild = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticRoots.staticDeepNested = default(DeepNestedRefs);
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Struct_array_of_nested_refs_scans_each_element()
        {
            NestedRefs[] array = new NestedRefs[5];
            object[] children = new object[5];
            for (int i = 0; i < children.Length; ++i)
            {
                children[i] = new object();
                array[i].leaf.payload = children[i];
            }
            GCHandle arrayPin = KeepAlive(array);
            GCHandle[] weaks = new GCHandle[children.Length];
            for (int i = 0; i < children.Length; ++i)
            {
                weaks[i] = TrackWeak(children[i]);
                children[i] = null;
            }
            try
            {
                FullCollect();
                for (int i = 0; i < weaks.Length; ++i)
                {
                    AssertSurvives(weaks[i]);
                }
            }
            finally
            {
                for (int i = 0; i < weaks.Length; ++i)
                {
                    FreeHandle(ref weaks[i]);
                }
                FreeHandle(ref arrayPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Value_type_without_references_does_not_pin_objects()
        {
            PrimitiveOnlyStruct[] array = new PrimitiveOnlyStruct[4];
            for (int i = 0; i < array.Length; ++i)
            {
                array[i].a = i;
                array[i].b = i * 10;
                array[i].c = i * 0.5;
            }
            object orphan = new object();
            GCHandle weak = TrackWeak(orphan);
            try
            {
                array = null;
                orphan = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Instance_struct_field_at_zero_offset_is_scanned()
        {
            var container = new RefContainer();
            object child = new object();
            container.nested.leaf.payload = child;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_struct_field_at_zero_offset_is_scanned()
        {
            object child = new object();
            GcStaticRoots.staticNested.leaf.payload = child;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticRoots.staticNested = default(NestedRefs);
                FreeHandle(ref weak);
            }
        }
    }
}
