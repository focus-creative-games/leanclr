using System;
using System.Runtime.InteropServices;
using GcTests.Fixtures;

namespace GcTests.Scan
{
    internal class TC_GC_Graph : GcTestCaseBase
    {
        [UnitTest]
        [GcUnitTest]
        public void Chain_length_four_survives_from_root()
        {
            object leaf = new object();
            GcNode root = BuildChain(4, leaf);
            GCHandle rootPin = KeepAlive(root);
            GCHandle weak = TrackWeak(leaf);
            try
            {
                leaf = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Chain_break_reclaims_tail()
        {
            object leaf = new object();
            GcNode root = BuildChain(4, leaf);
            GCHandle rootPin = KeepAlive(root);
            root.next.next = null;
            GCHandle weak = TrackWeak(leaf);
            try
            {
                leaf = null;
                FullCollect();
                AssertCollected(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Two_node_cycle_collectible_when_unrooted()
        {
            var a = new GcNode();
            var b = new GcNode();
            a.next = b;
            b.next = a;
            GCHandle weakA = TrackWeak(a);
            GCHandle weakB = TrackWeak(b);
            try
            {
                a = null;
                b = null;
                FullCollect();
                AssertCollected(weakA);
                AssertCollected(weakB);
            }
            finally
            {
                FreeHandle(ref weakA);
                FreeHandle(ref weakB);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Cycle_with_external_root_keeps_all_nodes()
        {
            var a = new GcNode();
            var b = new GcNode();
            a.next = b;
            b.next = a;
            GCHandle weakA = TrackWeak(a);
            GCHandle weakB = TrackWeak(b);
            try
            {
                GcStaticRoots.root = a;
                FullCollect();
                AssertSurvives(weakA);
                AssertSurvives(weakB);
            }
            finally
            {
                GcStaticRoots.root = null;
                FreeHandle(ref weakA);
                FreeHandle(ref weakB);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Self_reference_collectible_when_unrooted()
        {
            var node = new SelfReferencingNode();
            node.payload = new object();
            node.self = node;
            GCHandle weakNode = TrackWeak(node);
            GCHandle weakPayload = TrackWeak(node.payload);
            try
            {
                node = null;
                FullCollect();
                AssertCollected(weakNode);
                AssertCollected(weakPayload);
            }
            finally
            {
                FreeHandle(ref weakNode);
                FreeHandle(ref weakPayload);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Diamond_graph_marks_shared_leaf_once()
        {
            object leaf = new object();
            var left = new GcNode { data = leaf };
            var right = new GcNode { data = leaf };
            var root = new GcNode { next = left };
            left.next = right;
            GCHandle rootPin = KeepAlive(root);
            GCHandle weak = TrackWeak(leaf);
            try
            {
                leaf = null;
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Wide_bush_marks_all_children()
        {
            var root = new GcNode();
            object[] children = new object[16];
            GCHandle[] weaks = new GCHandle[children.Length];
            for (int i = 0; i < children.Length; ++i)
            {
                children[i] = new object();
                weaks[i] = TrackWeak(children[i]);
                var childNode = new GcNode { data = children[i] };
                childNode.next = root.next;
                root.next = childNode;
                children[i] = null;
            }
            GCHandle rootPin = KeepAlive(root);
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
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Deep_chain_reclaims_unreachable_suffix()
        {
            object keep = new object();
            object drop = new object();
            GcNode root = BuildChain(6, keep);
            GcNode cursor = root;
            while (cursor.next != null)
            {
                cursor = cursor.next;
            }
            cursor.next = new GcNode { data = drop };
            GCHandle rootPin = KeepAlive(root);
            GCHandle weakKeep = TrackWeak(keep);
            GCHandle weakDrop = TrackWeak(drop);
            try
            {
                keep = null;
                drop = null;
                cursor.next = null;
                FullCollect();
                AssertSurvives(weakKeep);
                AssertCollected(weakDrop);
            }
            finally
            {
                FreeHandle(ref weakKeep);
                FreeHandle(ref weakDrop);
                FreeHandle(ref rootPin);
            }
        }

        [UnitTest]
        [GcUnitTest]
        public void Repeated_collect_on_stable_graph_is_idempotent()
        {
            object leaf = new object();
            GcNode root = BuildChain(3, leaf);
            GCHandle rootPin = KeepAlive(root);
            GCHandle weak = TrackWeak(leaf);
            try
            {
                leaf = null;
                FullCollect();
                FullCollect();
                AssertSurvives(weak);
            }
            finally
            {
                FreeHandle(ref weak);
                FreeHandle(ref rootPin);
            }
        }
    }
}
