//--------------------------------------------------------------------
// HashMap.chug Unit Tests
// Author: Andrew Zhu Aday (azaday)
// Date: Spring 2024
// Want to see more information? Add the --verbose:3 (-v3) flag:
//    `chuck --chugin:HashMap.chug hashmap-test.ck -v3`
//--------------------------------------------------------------------

// Test Harness
public class T 
{
    fun static void println(string s) {
        chout <= s <= IO.nl();
    }

    fun static void err(string s) {
        cherr <= s <= IO.nl();
    }

    fun static void assert(int bool) {
        if (!bool) err("Assertion failed");
    }

    fun static void assert(int bool, string s) {
        if (!bool) err("Assertion failed: " + s);
    }

    fun static int feq(float a, float b) {
        return Math.fabs(a - b) < 0.0001;
    }
}

class Foo {
    0 => int x;
}

// Test all possible pairings of
// int / Object / string --> int / float / Object / string

Foo foo;
1337 => foo.x;
Foo bar;
1338 => bar.x;

fun void hm_test()
{
    HashMap hm;

    // Set & Get ============================================================
    hm.set(1, 2);
    T.assert(hm.getInt(1) == 2, "Set int -> int");
    hm.set(1, 3.0);
    T.assert(T.feq(hm.getFloat(1), 3.0), "Set int -> float");
    hm.set(1, foo);
    T.assert(hm.getObj(1) $ Foo == foo, "Set int -> Object");
    hm.set(1, "hello");
    T.assert(hm.getStr(1) == "hello", "Set int -> string");

    hm.set(foo, 2);
    T.assert(hm.getInt(foo) == 2, "Set Object -> int");
    hm.set(foo, 3.0);
    T.assert(T.feq(hm.getFloat(foo), 3.0), "Set Object -> float");
    hm.set(foo, foo);
    T.assert(hm.getObj(foo) $ Foo == foo, "Set Object -> Object");
    hm.set(foo, "hello");
    T.assert(hm.getStr(foo) == "hello", "Set Object -> string");

    hm.set("hello", 2);
    T.assert(hm.getInt("hello") == 2, "Set string -> int");
    hm.set("hello", 3.0);
    T.assert(T.feq(hm.getFloat("hello"), 3.0), "Set string -> float");
    hm.set("hello", foo);
    T.assert(hm.getObj("hello") $ Foo == foo, "Set string -> Object");
    hm.set("hello", "bye");
    T.assert(hm.getStr("hello") == "bye", "Set string -> string");

    // size ============================================================
    T.assert(hm.size() == 3, "size after 3 inserts");

    // Has ============================================================
    T.assert(hm.has(1), "has int");
    T.assert(hm.has(foo), "has Object");
    T.assert(hm.has("hello"), "has string");
    T.assert(!hm.has(2), "does not have int");
    T.assert(!hm.has(bar), "does not have Object");
    T.assert(!hm.has("bye"), "does not have string");

    // defaults ============================================================
    T.assert(hm.getInt(9999) == 0, "default int value for int key");
    T.assert(T.feq(hm.getFloat(9999), 0), "default float value for int key");
    T.assert(hm.getObj(9999) == null, "default Object value for int key");
    T.assert(hm.getStr(9999) == "", "default string value for int key");

    T.assert(hm.getInt(bar) == 0, "default int value for Object key");
    T.assert(T.feq(hm.getFloat(bar), 0), "default float value for Object key");
    T.assert(hm.getObj(bar) == null, "default Object value for Object key");
    T.assert(hm.getStr(bar) == "", "default string value for Object key");

    T.assert(hm.getInt("bye") == 0, "default int value for string key");
    T.assert(T.feq(hm.getFloat("bye"), 0), "default float value for string key");
    T.assert(hm.getObj("bye") == null, "default Object value for string key");
    T.assert(hm.getStr("bye") == "", "default string value for string key");

    // Delete & Clear & Refcounting ============================================================
    // bar as value is refcounted
    T.assert(Machine.refcount(bar) == 1, "initial bar refcount");
    hm.set(2, bar);
    T.assert(Machine.refcount(bar) == 2, "bar refcount after set as value");
    hm.set(2, foo);
    T.assert(Machine.refcount(bar) == 1, "bar refcount after set to foo");
    // bar as key is refcounted
    hm.set(bar, 2);
    T.assert(Machine.refcount(bar) == 2, "bar refcount after set as key");
    hm.del(bar);
    T.assert(Machine.refcount(bar) == 1, "bar refcount after del");
    // size and refcounts reset after clearing
    hm.clear();
    T.assert(hm.size() == 0, "size after clear");
    T.assert(Machine.refcount(bar) == 1, "bar refcount after clear");
    T.assert(Machine.refcount(foo) == 1, "foo refcount after clear");

    hm.set(bar, 2);
    hm.set(foo, 3);
    T.assert(Machine.refcount(bar) == 2, "bar refcount before scope");
    T.assert(Machine.refcount(foo) == 2, "foo refcount before scope");
}

hm_test();

// destructor called, bar and foo refcounts should be 1
T.assert(Machine.refcount(bar) == 1, "bar refcount after scope");
T.assert(Machine.refcount(foo) == 1, "foo refcount after scope");


// <<< Machine.refcount(hm) >>>;
<<< Machine.refcount(bar), Machine.refcount(foo) >>>;