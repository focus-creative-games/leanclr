//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.Bugs
//{
//    internal class IncrementalGCMdArrayLeak : GeneralTestCaseBase
//    {
//        public class Node
//        {
//            public const int SIZE = 60;
//            public Vector3 position;
//            public int i;
//            public int j;
//        }

//        [UnitTest]
//        public void TestMdArrayAccess()
//        {
//            var Grid = new Node[Node.SIZE, Node.SIZE];
//            for (int i = 0; i < Node.SIZE; i++)
//            {
//                for (int j = 0; j < Node.SIZE; j++)
//                {
//                    Grid[i, j] = new Node()
//                    {
//                        position = new Vector3(i * Node.SIZE, j, 0),
//                        i = i,
//                        j = j,
//                    };
//                }
//            }
//            for (int i = 0; i < Node.SIZE; i++)
//            {
//                for (int j = 0; j < Node.SIZE; j++)
//                {
//                    var n = Grid[i, j];
//                    Assert.Equal(i, n.i);
//                    Assert.Equal(j, n.j);
//                }
//            }
//        }
//    }
//}
