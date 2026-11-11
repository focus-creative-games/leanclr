extern alias GenericArguments;
using System;
using GenericAotLibrary;
using ForeignReference = GenericArguments::GenericAotArguments.ReferenceValue;

namespace AotTests
{
    public class TC_ReferenceSharing
    {
        public class Unregistered { }
        [UnitTest] public void SelectAcrossAssemblies()
        {
            var foreign = new ForeignReference { Marker = 5 };
            Assert.True(SharedReferences.Select<string, ForeignReference>("a", "b", foreign, 3) == "a");
            Assert.True(SharedReferences.Select<ForeignReference, string>(foreign, null, "s", 3) == foreign);
            Assert.True(SharedReferences.Select<object, string>(foreign, null, "s", -3) == null);
            Assert.True(SharedReferences.Select<string[], object>(new[] { "array" }, null, foreign, 3)[0] == "array");
        }
        [UnitTest] public void ClassAndMethodContexts()
        {
            var foreign = new ForeignReference();
            Assert.True(new SharedReferenceHolder<string>().Second<ForeignReference>("s", foreign) == foreign);
            Assert.True(new SharedReferenceHolder<ForeignReference>().Second<string>(foreign, "s") == "s");
        }
        [UnitTest] public void SharedErrors()
        {
            int caught = 0;
            try { SharedReferences.Select<string, object>("s", null, null, 0); } catch (DivideByZeroException) { ++caught; }
            try { SharedReferences.Select<ForeignReference, string>(null, null, "s", 0); } catch (DivideByZeroException) { ++caught; }
            Assert.True(caught == 2);
        }
        [UnitTest] public void ReflectionAndUnregistered()
        {
            var definition = typeof(Functions).GetMethod("Identity");
            Assert.True((string)definition.MakeGenericMethod(typeof(string)).Invoke(null, new object[] { "reflect" }) == "reflect");
            object value = new Unregistered();
            Assert.True(definition.MakeGenericMethod(typeof(Unregistered)).Invoke(null, new object[] { value }) == value);
        }
        [UnitTest] public void TypeDependentSpecializations()
        {
            Assert.True(SharedReferences.Context<string>() == 7 && SharedReferences.Context<ForeignReference>() == 11);
            var a = new Box<string>("s");
            var b = new Box<ForeignReference>(new ForeignReference());
            Box<string>.Static = "sharing-static";
            Box<ForeignReference>.Static = b.Value;
            Assert.True(a.Read() == "s" && b.Read() == b.Value && Box<string>.Static == "sharing-static" && Box<ForeignReference>.Static == b.Value);
        }
        [UnitTest] public void LoopAndNull()
        {
            var reference = new ForeignReference();
            Assert.True(SharedReferences.Loop<string>("s", null, 8, 0) == "s");
            Assert.True(SharedReferences.Loop<ForeignReference>(reference, null, 8, 1) == null);
            Assert.True(SharedReferences.Loop<object>(reference, null, 8, 0) == reference);
        }
    }
}
