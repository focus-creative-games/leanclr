
public class GenericClassWithoutAnyInst<A> where A:new()
{
    public void GCWAI_Param(A p)
    {

    }
    public static void Static_GCWAI_Param(A p)
    {

    }

    public A GCWAI_Return()
    {
        return new A();
    }

    public A GCWAI_Mix(A p)
    {
        p =  new A();
        return p;
    }
}

public class GenericClassWithOnlyClassInst<A> where A : new()
{
    public void GCWOCI_Param(A p)
    {

    }
    public A GCWOCI_Return()
    {
        return new A();
    }

    public static A Static_GCWOCI_Mix(A p)
    {
        p = new A();
        return p;
    }

    public A GCWOCI_Mix(A p)
    {
        p = new A();
        return p;
    }
}

public class GenericClassWithOnlyStructInst<A> where A : new()
{
    public void GCWOSI_Param(A p)
    {

    }
    public A GCWOSI_Return()
    {
        return new A();
    }

    public static A Static_GCWOSI_Mix(A p)
    {
        p = new A();
        return p;
    }

    public A GCWOSI_Mix(A p)
    {
        p = new A();
        return p;
    }
}

public class GenericClassWithAllInst<A> where A : new()
{
    public void GCWAllI_Param(A p)
    {

    }
    public A GCWAllI_Return()
    {
        return new A();
    }

    public static A Static_GCWAllI_Mix(A p)
    {
        p = new A();
        return p;
    }

    public A GCWAllI_Mix(A p)
    {
        p = new A();
        return p;
    }

}


public class InstFullGenerics
{
    public class Word
    {
        public int x;
    }

    public struct Bee
    {
        public int x;
    }

    public void Init()
    {
        {
            var c = new GenericClassWithOnlyClassInst<Word>();
            c.GCWOCI_Param(new Word());
            c.GCWOCI_Return();
            c.GCWOCI_Mix(new Word());
            GenericClassWithOnlyClassInst<Word>.Static_GCWOCI_Mix(new Word());
        }
        {
            var c = new GenericClassWithOnlyStructInst<Bee>();
            c.GCWOSI_Param(new Bee());
            c.GCWOSI_Return();
            c.GCWOSI_Mix(new Bee());
            GenericClassWithOnlyStructInst<Bee>.Static_GCWOSI_Mix(new Bee());
        }
        {
            var c = new GenericClassWithAllInst<Word>();
            c.GCWAllI_Param(new Word());
            c.GCWAllI_Return();
            c.GCWAllI_Mix(new Word());
            GenericClassWithAllInst<Word>.Static_GCWAllI_Mix(new Word());
        }
        {
            var c = new GenericClassWithAllInst<Bee>();
            c.GCWAllI_Param(new Bee());
            c.GCWAllI_Return();
            c.GCWAllI_Mix(new Bee());
            GenericClassWithAllInst<Bee>.Static_GCWAllI_Mix(new Bee());
        }
    }
}