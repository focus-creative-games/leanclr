using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Roots
{
    internal class TC_GC_Roots : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Static_object_field_is_a_gc_root()
        {
            GcStaticRoots.root = new object();
            object local = new object();
            GCHandle weak = TrackWeak(local);
            try
            {
                local = null;
                FullCollect();
                AssertCollected(weak);
                Assert.NotNull(GcStaticRoots.root);
            }
            finally
            {
                FreeHandle(ref weak);
                GcStaticRoots.root = null;
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_string_field_is_a_gc_root()
        {
            GcStaticRoots.staticString = "static-gc-root";
            GCHandle weak = TrackWeak(GcStaticRoots.staticString);
            try
            {
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticRoots.staticString = null;
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_nested_value_type_field_is_a_gc_root()
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

        [UnitTest]
        [GcUnitTest]
        public void Static_container_field_keeps_children()
        {
            var container = new RefContainer();
            object child = new object();
            container.field0 = child;
            GcStaticRoots.staticContainer = container;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                container = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticRoots.staticContainer = null;
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Static_reference_array_field_is_a_gc_root()
        {
            object element = new object();
            GcStaticRoots.staticArray = new object[] { element, null };
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                GcStaticRoots.staticArray = null;
                FreeHandle(ref weak);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Clearing_static_root_allows_reachability_to_drop()
        {
            object child = new object();
            GcStaticRoots.root = child;
            GCHandle weak = TrackWeak(child);
            try
            {
                child = null;
                GcStaticRoots.root = null;
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
        public void Instance_field_chain_from_pinned_root()
        {
            var container = new RefContainer();
            object child = new object();
            container.field0 = child;
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
        public void Inherited_instance_field_reachable_from_pinned_derived_root()
        {
            var derived = new GcDerivedWithRef();
            object baseChild = new object();
            derived.baseField = baseChild;
            GCHandle derivedPin = KeepAlive(derived);
            GCHandle weak = TrackWeak(baseChild);
            try
            {
                baseChild = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref derivedPin);
            }
        }
    }
}
