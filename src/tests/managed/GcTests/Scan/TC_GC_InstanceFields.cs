using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Scan
{
    internal class TC_GC_InstanceFields : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Direct_reference_field_child_collected_when_field_cleared()
        {
            var container = new RefContainer();
            object child = new object();
            container.field0 = child;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(child);
            try
            {
                container.field0 = null;
                child = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Container_root_keeps_both_reference_fields_alive()
        {
            var container = new RefContainer();
            object child0 = new object();
            object child1 = new object();
            container.field0 = child0;
            container.field1 = child1;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak0 = TrackWeak(child0);
            GCHandle weak1 = TrackWeak(child1);
            try
            {
                child0 = null;
                child1 = null;
                FullCollect();
                AssertSurvives(weak0);
                AssertSurvives(weak1);
            }
            finally
            {
                FreeHandle(ref weak0);
                FreeHandle(ref weak1);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Unrooted_container_reclaims_all_children()
        {
            var container = new RefContainer();
            object child0 = new object();
            object child1 = new object();
            container.field0 = child0;
            container.field1 = child1;
            GCHandle weak0 = TrackWeak(child0);
            GCHandle weak1 = TrackWeak(child1);
            try
            {
                container = null;
                child0 = null;
                child1 = null;
                FullCollect();
                AssertCollected(weak0);
                AssertCollected(weak1);
            }
            finally
            {
                FreeHandle(ref weak0);
                FreeHandle(ref weak1);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void String_field_child_collected_when_field_cleared()
        {
            var container = new RefContainer();
            string value = UniqueString("gc-string-field");
            container.text = value;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(value);
            try
            {
                container.text = null;
                value = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void String_field_with_container_root_survives()
        {
            var container = new RefContainer();
            string value = UniqueString("gc-string-survive");
            container.text = value;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(value);
            try
            {
                value = null;
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
        public void Inherited_base_reference_field_is_scanned()
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

        [UnitTest]
        [GcUnitTest]
        public void Derived_reference_field_is_scanned()
        {
            var derived = new GcDerivedWithRef();
            object derivedChild = new object();
            derived.derivedField = derivedChild;
            GCHandle derivedPin = KeepAlive(derived);
            GCHandle weak = TrackWeak(derivedChild);
            try
            {
                derivedChild = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref derivedPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Nested_struct_reference_field_is_scanned()
        {
            var container = new RefContainer();
            object nestedChild = new object();
            container.nested.leaf.payload = nestedChild;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(nestedChild);
            try
            {
                nestedChild = null;
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
        public void Nested_struct_reference_cleared_then_collected()
        {
            var container = new RefContainer();
            object nestedChild = new object();
            container.nested.leaf.payload = nestedChild;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(nestedChild);
            try
            {
                container.nested.leaf.payload = null;
                nestedChild = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Array_field_on_object_is_scanned()
        {
            var container = new RefContainer();
            object element = new object();
            container.arrayField = new object[] { element, null };
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(element);
            try
            {
                element = null;
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
        public void Primitive_only_object_has_no_reference_children()
        {
            var container = new PrimitiveOnlyContainer();
            container.a = 42;
            container.b = 99;
            container.c = 3.14;
            GCHandle weak = TrackWeak(container);
            try
            {
                container = null;
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
        public void Replaced_reference_field_collects_old_child()
        {
            var container = new RefContainer();
            object oldChild = new object();
            container.field0 = oldChild;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(oldChild);
            try
            {
                container.field0 = new object();
                oldChild = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Null_reference_fields_do_not_keep_unreachable_objects()
        {
            var container = new RefContainer();
            object orphan = new object();
            container.field0 = null;
            container.field1 = null;
            GCHandle containerPin = KeepAlive(container);
            GCHandle weak = TrackWeak(orphan);
            try
            {
                orphan = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref containerPin);
            }
        }
    }
}
