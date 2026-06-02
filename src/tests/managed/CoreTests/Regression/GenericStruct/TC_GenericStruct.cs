using test;
using System.Diagnostics;
using System.Text;

namespace Tests.Bugs
{
    /// <summary>
    /// 数值计算公式
    /// </summary>
    public class TC_GenericStruct : GeneralTestCaseBase
    {
        public struct FixedArray8<T>
        {
            T item0;
            T item1;
            T item2;
            T item3;
            T item4;
            T item5;
            T item6;
            T item7;


            public int Capacity => 8;
            public int Length { get; private set; }

            public FixedArray8(int length)
            {
                Length = 0;
                item0 = default;
                item1 = default;
                item2 = default;
                item3 = default;
                item4 = default;
                item5 = default;
                item6 = default;
                item7 = default;

                if (length > Capacity)
                {
                    throw new System.IndexOutOfRangeException();
                }

                Length = length;
            }

            public T GetValue(int index)
            {
                switch (index)
                {
                case 0: return item0;
                case 1: return item1;
                case 2: return item2;
                case 3: return item3;
                case 4: return item4;
                case 5: return item5;
                case 6: return item6;
                case 7: return item7;
                default: return default;
                }
            }
        }

        /// <summary>
        /// 技能数值计算时使用的结果权重池
        /// </summary>
        public struct ResultWeights
        {
            /// <summary>
            /// 权重池所在的标准体验表配置id
            /// </summary>
            public int standardCfgId;

            /// <summary>
            /// 权重池的总调整值上限
            /// </summary>
            public int adjustLimit;

            /// <summary>
            /// 基础权重列表
            /// </summary>
            public FixedArray8<int> baseWeights;

            /// <summary>
            /// 权重上限列表
            /// </summary>
            public FixedArray8<int> maxWeights;

            /// <summary>
            /// 权重下限列表
            /// </summary>
            public FixedArray8<int> minWeights;

            /// <summary>
            /// 调整后的池子
            /// </summary>
            public FixedArray8<int> adjustedWeights;

            public ResultWeights(string data, int cfgId)
            {
                standardCfgId = cfgId;
                adjustLimit = 0;

                //UnityEngine.Debug.Log($"1 ResultWeights data {data} cfgId {cfgId}");
                baseWeights = new FixedArray8<int>();
                //UnityEngine.Debug.Log($"2 ResultWeights data {data} cfgId {cfgId}");
                maxWeights = new FixedArray8<int>();
                //UnityEngine.Debug.Log($"3 ResultWeights data {data} cfgId {cfgId}");
                minWeights = new FixedArray8<int>();
                //UnityEngine.Debug.Log($"4 ResultWeights data {data} cfgId {cfgId}");
                adjustedWeights = new FixedArray8<int>();
            }
        }

        [UnitTest]
        public void Test()
        {
            var r = new ResultWeights("0,1,0|0,100,0", 123);
            Assert.Equal(123, r.standardCfgId);
        }
    }
}