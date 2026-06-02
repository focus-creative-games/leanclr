//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.Bugs
//{
//    public class ObjectPool<T> where T : class
//    {
//        private const int DEFAULT_RESERVED = 16;
//        private const int DEFAULT_CAPCITY = 32;

//        private readonly Stack<T> m_Stack;
//        private readonly Func<T> m_ActionCreated;
//        private readonly Action<T> m_ActionOnGet;
//        private readonly Action<T> m_ActionOnRelease;
//        private int m_Capcity;

//        public int countAll { get; private set; }

//        public ObjectPool(Func<T> actionCreated) : this(actionCreated, null, null)
//        {

//        }

//        public ObjectPool(Func<T> actionCreated, Action<T> actionOnGet, Action<T> actionOnRelease, int reserved = DEFAULT_RESERVED, int capcity = DEFAULT_CAPCITY)
//        {
//            m_Stack = new Stack<T>();
//            m_ActionCreated = actionCreated;
//            m_ActionOnGet = actionOnGet;
//            m_ActionOnRelease = actionOnRelease;
//            m_Capcity = capcity;

//            //预创建
//            if (null != m_ActionCreated)
//            {
//                for (int i = 0; i < reserved && i < m_Capcity; i++)
//                {
//                    var entity = m_ActionCreated();
//                    countAll++;
//                    Release(entity);
//                }
//            }
//        }

//        public void Release(T element)
//        {
//            if (m_ActionOnRelease != null)
//                m_ActionOnRelease(element);
//            if (m_Stack.Count < m_Capcity)
//            {
//                m_Stack.Push(element);
//            }
//        }
//    }

//    public class Data
//    {
//        public int ID;
//        public string Name;
//    }

//    class Bug_Delegate_2022_10_13 : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void static_action()
//        {
//            var m_pool = new ObjectPool<Data>(OnCreate, null, OnRelease);
//        }

//        // Update is called once per frame
//        public static Data OnCreate()
//        {
//            return new Data();
//        }

//        public static void OnRelease(Data data)
//        {

//        }
//    }
//}
