using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Roots
{
    internal struct RefNode
    {
        public object next;
        public int id;
    }

    internal class StaticRootHolder
    {
        public static object static_ref;
        public static RefNode static_value_node;
    }

    internal class InstanceRootContainer
    {
        public RefNode instance_value_node;
        public object instance_ref;
    }

    internal class TC_GC_Roots : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Static_field_root()
        {
            StaticRootHolder.static_ref = new object();
            object local = new object();
            GCHandle weak = GCHandle.Alloc(local, GCHandleType.Weak);
            try
            {
                local = null;
                FullCollect();
                Assert.Null(weak.Target);
                Assert.NotNull(StaticRootHolder.static_ref);
            }
            finally
            {
                if (weak.IsAllocated)
                {
                    weak.Free();
                }
                StaticRootHolder.static_ref = null;
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Nested_value_type_instance_refs()
        {
            var container = new InstanceRootContainer();
            container.instance_value_node.next = new object();
            GCHandle weak = GCHandle.Alloc(container.instance_value_node.next, GCHandleType.Weak);
            try
            {
                container.instance_value_node.next = null;
                FullCollect();
                Assert.Null(weak.Target);
            }
            finally
            {
                if (weak.IsAllocated)
                {
                    weak.Free();
                }
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Nested_value_type_static_refs()
        {
            StaticRootHolder.static_value_node.next = new object();
            GCHandle weak = GCHandle.Alloc(StaticRootHolder.static_value_node.next, GCHandleType.Weak);
            try
            {
                StaticRootHolder.static_value_node.next = null;
                FullCollect();
                Assert.Null(weak.Target);
            }
            finally
            {
                if (weak.IsAllocated)
                {
                    weak.Free();
                }
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Reference_array_elements()
        {
            object[] array = new object[4];
            array[0] = new object();
            GCHandle weak = GCHandle.Alloc(array[0], GCHandleType.Weak);
            try
            {
                array[0] = null;
                FullCollect();
                Assert.Null(weak.Target);
            }
            finally
            {
                if (weak.IsAllocated)
                {
                    weak.Free();
                }
            }
        }
    }
}
