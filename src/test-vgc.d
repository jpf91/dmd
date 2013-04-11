module testvgc;


class A {}
struct B{};

void test(string a, string b)
{
    auto s = a ~ b;
    s ~= "!!!";
}

int delegate(int) closure()
{
    int a;
    
    int add(int b)
    {
        return a + b;
    }
    
    return &add;
}

void main()
{
    auto a = new A();
    auto b = new B();
    auto c = new int();
    auto d = new int[](42);
    
    assert(false, "msg");
    assert(d.length);
    
    int[] arr;
    arr.length = 42;
    
    test("a", "b");
    
    auto lit = [1,2,3];
    int[3] lit2 = [];
    
    //What about templates?
    //auto aa = ["a":9, "b":3];
    
    enum X
    {
        A
    }
    X x;
    final switch(x)
    {
        case X.A:
            break;
    }
    
    int cint;
    void noclosure(int a)
    {
        cint += a;
    }
    closureHelper(&noclosure);
    
    char[] arr2;
    arr2.sort;
    
    auto arr3 = arr2.dup;
    auto arr4 = arr3.idup;
}

void closureHelper(/*scope*/ void delegate(int a) del)
{
    del(42);
}

unittest
{
    assert(false);
    assert(false, "");
}
