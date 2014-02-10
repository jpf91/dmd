/*
REQUIRED_ARGS: -vgc
TEST_OUTPUT:
---
compilable/nogc_warn.d(44): vgc: Array literals cause gc allocation
compilable/nogc_warn.d(45): vgc: Array literals cause gc allocation
compilable/nogc_warn.d(47): vgc: Array literals cause gc allocation
compilable/nogc_warn.d(53): vgc: 'new' causes gc allocation
compilable/nogc_warn.d(54): vgc: 'new' causes gc allocation
compilable/nogc_warn.d(55): vgc: 'new' causes gc allocation
compilable/nogc_warn.d(56): vgc: 'new' causes gc allocation
compilable/nogc_warn.d(61): vgc: 'delete' requires gc
compilable/nogc_warn.d(66): vgc: 'delete' requires gc
compilable/nogc_warn.d(71): vgc: 'delete' requires gc
compilable/nogc_warn.d(78): vgc: Associative array literals cause gc allocation
compilable/nogc_warn.d(79): vgc: Associative array literals cause gc allocation
compilable/nogc_warn.d(85): vgc: Setting 'length' may cause gc allocation
compilable/nogc_warn.d(88): vgc: Setting 'length' may cause gc allocation
compilable/nogc_warn.d(89): vgc: Setting 'length' may cause gc allocation
compilable/nogc_warn.d(94): vgc: Concatenation may cause gc allocation
compilable/nogc_warn.d(95): vgc: Concatenation may cause gc allocation
compilable/nogc_warn.d(98): vgc: Concatenation may cause gc allocation
compilable/nogc_warn.d(99): vgc: Concatenation may cause gc allocation
compilable/nogc_warn.d(104): vgc: Using closure causes gc allocation
compilable/nogc_warn.d(122): vgc: 'sort' may cause gc allocation
compilable/nogc_warn.d(123): vgc: 'sort' may cause gc allocation
compilable/nogc_warn.d(124): vgc: 'sort' may cause gc allocation
compilable/nogc_warn.d(125): vgc: 'sort' may cause gc allocation
compilable/nogc_warn.d(131): vgc: 'dup' causes gc allocation
compilable/nogc_warn.d(132): vgc: 'dup' causes gc allocation
compilable/nogc_warn.d(138): vgc: 'dup' causes gc allocation
compilable/nogc_warn.d(139): vgc: 'dup' causes gc allocation
compilable/nogc_warn.d-mixin-145(145): vgc: 'new' causes gc allocation
compilable/nogc_warn.d(151): vgc: Indexing an associative array may cause gc allocation
---
*/

enum int[] enumliteral = [1,2,3,4];

void testArrayLiteral()
{
    void helper(int[] a){}

    int[] allocate2 = [1,2,4];
    int[] e = enumliteral;

    helper([1,2,3]);
}

struct Struct {}
void testNew()
{
    auto obj = new Object();
    auto s = new Struct();
    auto s2 = new Struct[5];
    auto i = new int[3];
}

void testDelete(Struct* instance)
{
    delete instance;
}

void testDelete(void* instance)
{
    delete instance;
}

void testDelete(Object instance)
{
    delete instance;
}

enum aaLiteral = ["test" : 0];
void testAA()
{
    int[string] aa;
    aa = ["test" : 0];
    aa = aaLiteral;
}

void testLength()
{
    string s;
    s.length = 100;

    int[] arr;
    arr.length += 1;
    arr.length -= 1;
}

void testConcat(string sin)
{
    sin ~= "test";
    string s2 = sin ~ "test";

    int[] arr;
    arr ~= 1;
    arr ~= arr;
}

void closureHelper2(void delegate() d) {}

void testClosure2()
{
    int a;

    void del1()
    {
        a++;
    }

    closureHelper2(&del1);
}

void testSort()
{
    int[] x;
    char[] c;
    wchar[] w;
    dchar[] d;
    x.sort;
    c.sort;
    w.sort;
    d.sort;
}

void testIdup()
{
    int[] x;
    auto x2 = x.idup;
    auto s2 = "test".idup;
}

void testDup()
{
    int[] x;
    auto x2 = x.dup;
    auto s2 = "test".dup;
}

// mixins
void testMixin()
{
    mixin("auto a = new " ~ int.stringof ~ " ();");
}

void testAAIndex()
{
    int[string] aa;
    aa["test"] = 0;
}
