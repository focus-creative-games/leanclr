using System.Collections.Generic;
using System.Reflection;

namespace Tests.Bugs
{
    class InflateGenericClassVTable
    {
        public abstract class SerializedDictionary<K, V, SK, SV> : Dictionary<K, V>
        {
            private List<SK> m_Keys = new List<SK>();

            private List<SV> m_Values = new List<SV>();

            public abstract SK SerializeKey(K key);

            public abstract SV SerializeValue(V value);

            public abstract K DeserializeKey(SK serializedKey);

            public abstract V DeserializeValue(SV serializedValue);
        }

        public class SerializedDictionary<K, V> : SerializedDictionary<K, V, K, V>
        {
            public override K SerializeKey(K key)
            {
                return key;
            }

            public override V SerializeValue(V val)
            {
                return val;
            }

            public override K DeserializeKey(K key)
            {
                return key;
            }

            public override V DeserializeValue(V val)
            {
                return val;
            }
        }

        [UnitTest]
        public void InflateGenericClassVTableMethodWhenMethodInheritFromBaseGenericClassWithDifferentGenericArguments()
        {
            var dict = new SerializedDictionary<string, int>();
            MethodInfo m = dict.GetType().GetMethod("SerializeKey");
            Assert.Equal(typeof(string), m.ReturnType);
        }
    }
}
