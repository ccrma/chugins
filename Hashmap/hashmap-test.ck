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

    fun static print(string a[]) {
        for (string s : a) {
            chout <= s <= " ";
        }
        chout <= IO.nl();
    }
}

class Foo {
    9999 => int x;
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

    // Type ============================================================
    T.assert(hm.Type_None == 0, "Type_None");
    T.assert(hm.Type_Int == 1, "Type_Int");
    T.assert(hm.Type_Float == 2, "Type_Float");
    T.assert(hm.Type_Obj == 3, "Type_Obj");
    T.assert(hm.Type_Str == 4, "Type_Str");

    T.assert(hm.type(1337) == hm.Type_None, "missing key has none type");
    hm.set(1337, 1);
    T.assert(hm.type(1337) == hm.Type_Int, "key has int type");
    hm.set(1337, 1.);
    T.assert(hm.type(1337) == hm.Type_Float, "changing key to float type");
    hm.set(1336, "what");
    T.assert(hm.type(1336) == hm.Type_Str, "key to string type");
    hm.set(1336, new Foo);
    T.assert(hm.type(1336) == hm.Type_Obj, "change key to object type");
    T.assert((hm.getObj(1336) $ Foo).x == 9999, "object stored correctly");
    hm.del(1336);
    T.assert(hm.type(1336) == hm.Type_None, "deleting key sets type to none");
    T.assert(hm.type("hello") == hm.Type_Str, "type of string key");
    T.assert(hm.type(foo) == hm.Type_Str, "type of obj key");

    // string copying ============================================================
    "what" => string key;
    hm.set(key, 100);
    hm.set("a", key);
    key + " x2" => key;
    T.assert(hm.getStr("a") == "what", "changing string reference doesnt change hm value");
    T.assert(hm.has("what"), "string key changed by reference");

    // keys ============================================================
    HashMap hm2;
    Foo obj2;
    T.assert(hm2.intKeys().size() == 0, "empty map int keys");
    T.assert(hm2.strKeys().size() == 0, "empty map str keys");
    T.assert(hm2.objKeys().size() == 0, "empty map obj keys");
    hm2.set(1, "1");
    hm2.set("2", 2.0);
    hm2.set(obj2, "what");
    T.assert(hm2.intKeys().size() == 1, "map int keys");
    T.assert(hm2.strKeys().size() == 1, "map str keys");
    T.assert(hm2.objKeys().size() == 1, "map obj keys");

    T.assert(hm2.intKeys()[0] == 1, "map int key");
    T.assert(hm2.strKeys()[0] == "2", "map str key");
    T.assert(hm2.objKeys()[0] == obj2, "map obj key");
    T.assert(Type.of(hm2.objKeys()[0]) == Type.of(obj2), "map obj key type equals");

    hm2.clear();
    T.assert(hm2.intKeys().size() == 0, "cleared map int keys");
    T.assert(hm2.strKeys().size() == 0, "cleared map str keys");
    T.assert(hm2.objKeys().size() == 0, "cleared map obj keys");

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

    // =================================================================
    // 1. given json string, create a hashmap
    // HashMap.fromJson(string json) @=> HashMap hm;
    // 2. given a hashmap, return a json string
    // hm.toJson() => string json;

    // T.assert(HashMap.fromJson("invalid json") == null, "invalid json returns null obj");
    <<< "ACTUAL JSON TEST ---------" >>>;

    {
        "
        {
            \"gregg\": [],
            \"andrew\": [
                1, 2, null,
                {
                    \"key\": 1337
                }
                false, true, \"string_value\",
                1.234
            ],
            \"obj\": {
                \"test\": 1.123456789,
                \"test2\": { \"1\": 10 }
            }
        }
        " => string json;
        HashMap.fromJson(json) @=> HashMap@ json_hm;

        <<< "1:------------" >>>;
        json_hm.set("hello", new StifKarp);
        <<< json_hm.toJson() >>>;

        <<< "2:-------------" >>>;
        HashMap.fromJson(json_hm.toJson()) @=> HashMap json_json_hm;
        json_json_hm.set("foo", "bar");
        HashMap new_hashmap;
        // new_hashmap.set("hello", "yooooo");
        new_hashmap.set(0, 99);
        new_hashmap.set(1, 100);
        new_hashmap.set(4, 99);
        new_hashmap.set(new SinOsc, 99);
        json_json_hm.set("new hashmap", new_hashmap);
        <<< json_json_hm.toJson() >>>;
        <<< "YOOO", (json_json_hm.get("andrew").get(3)).getInt("key") >>>;
    }
    // for (auto key : json_hm.strKeys()) {
    //     json_hm.get(key) @=> HashMap hm;

    //     <<< key, hm.size() >>>;
    //     if (key == "andrew") {
    //         for (int i; i < hm.size(); i++) {
    //             hm.type(i) => int type;
    //             if (type == hm.Type_Int) <<< hm.getInt(i) >>>;
    //             if (type == hm.Type_Float) <<< hm.getFloat(i) >>>;
    //             if (type == hm.Type_Obj) <<< hm.getObj(i) >>>;
    //             if (type == hm.Type_Str) <<< hm.getStr(i) >>>;
    //         }
    //     }
    // }
}

hm_test();

// destructor called, bar and foo refcounts should be 1
T.assert(Machine.refcount(bar) == 1, "bar refcount after scope");
T.assert(Machine.refcount(foo) == 1, "foo refcount after scope");
