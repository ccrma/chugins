# HashMap.chug

A generic hashmap data structure that works with any combination of `int`/`string`/`Object` data types.

## Usage

HashMap is designed to be heterogenous, i.e. a single hashmap object can store multiple key/value type combinations.

```c
HashMap hm;
SinOsc osc;

// set key-value pairs for any combination of int/str/obj --> int/float/str/obj
hm.set(1, 2.9);
hm.getInt(1);  // returns 2.9
hm.set("sin", osc);
hm.getObj("sin") $ SinOsc; // returns osc

// check if an element (key) exists
hm.has(1); // true
hm.has("sin"); // true
hm.has(osc); // false. osc is a value, not key

// delete elements
hm.del("sin");
hm.has("sin"); // return false
hm.getObj("sin"); // returns null

// iterating over keys
hm.intKeys() @=> int int_keys[];
hm.strKeys() @=> string str_keys[];
hm.objKeys() @=> Object obj_keys[];

// checking type of value under a key
hm.type(1) => int value_type;
if (value_type == hm.Type_None) {
    // key 1 does not exist
}
if (value_type == hm.Type_Int) {
    // key 1 maps to an int
    hm.getInt(1);
}
if (value_type == hm.Type_Float) {
    // key 1 maps to a float
    hm.getFloat(1);
}
if (value_type == hm.Type_Str) {
    // key 1 maps to a string
    hm.getStr(1);
}
if (value_type == hm.Type_Obj) {
    // key 1 maps to an obj
    hm.getObj(1);
}

// clear 
hm.clear()

// json reading and writing
HashMap.fromJson("{ \"json\": \"string\" }") @=> HashMap@ json_hm;
HashMap.fromJsonFile("./json.json") @=> HashMap@ json_hm_from_file;
json_hm.toJson(); // generate a json string

```

## JSON Parsing and Dumping

JSON strings and files can be parsed into a HashMap via `HashMap.fromJson(string json)` and `HashMap.fromJsonFile(string filepath)`. Parsing rules include:
- JSON Objects {...} are converted into a HashMap with matching string keys
- JSON Arrays are converted into a HashMap with integer keys 0, 1, ...
- JSON true and false are converted into chuck integers 1 and 0, respectively
- JSON null is converted into a `null` chuck Object
- JSON numbers with a decimal `.` are converted to chuck floats
- JSON numbers without a decimal are converted to chuck ints

HashMap also defines 2 convenience functions (`.get(int)` and `.get(string`) which return a HashMap. These are useful for traversing nested JSON structures:

```c
"
{
    \"array\": [
        1,
        2,
        { \"key\": \"value\" }
    ]
}
" => string json_string;

HashMap.fromJson(json_string) @=> HashMap hm;
<<< hm.get("array").get(2).getStr("key") >>>; // prints "value"
```

HashMap objects can also be converted to a JSON string via `hm.toJson()`. Conversion uses the following rules:
- If the HashMap has *only* int keys, and they begin at 0, return JSON array.
- If there are *any* string keys, ignore all int keys and parse as a JSON object.
- Ignore all Object keys.
- If any values are a HashMap, parse into a json string recursively.
- Object values are converted to their classname string.

For Example:

```c
HashMap hm;
hm.set("key0", 1);
hm.set("key1", 2.3);
hm.set("key2", new SinOsc);

HashMap nested_hm;
nested_hm.set(0, true);
nested_hm.set(1, 4.5);

hm.set("key3", nested_hm);

<<< hm.toJson() >>>;

/* outputs: 
"{"key0": 1, "key1": 2.300000, "key3": [1, 4.500000], "key2": "SinOsc"}" :(string)
*/
```

Note that because the hashing algorithm is pseudo-random, the order of key-value pairs in the json string conversion is *not* deterministic.

## Details

The underlying implementation is provided by [hashmap.c](https://github.com/tidwall/hashmap.c). It uses open addressing (i.e. probing a contiguous array for collision resolution rather than chaining.) As such it is cache friendly and iteration is relatively fast.

Hashmap currently uses [xxHash](https://github.com/Cyan4973/xxHash) as its hashing function. Perhaps in the future we can add others, or the option for users to supply their own.

Hashmap uses a grow-factor of 0.60, and a shrink-factor of 0.10.

Objects are hashed by reference (using their pointer address). Two different objects with the same values will NOT resolve to the same hash. 

See `hashmap-test.ck` for (relatively) comprehensive unit tests. Refcounting is accounted for, at least for the current test coverage.

## Known Bugs

If hashmaps form a cyclic dependency (e.g. 2 hashmaps point to each other) the json functions `HashMap.toJson()` will crash.

Calling `toJson()` on a HashMap with a `null` value causes a segfault.

## Benchmarks

`hm_bench.cpp` contains benchmark hashmap tests. Performance is similar or better than `std::unordered_map`

## Next Steps?

- Support other hash functions
- User-defined hash functions
- user-defined grow / shrink factors
- If/when ChucK adds generics Hashmap will either become obsolete or need to be reworked
- add an iterator (requires adding `HashItem` and `HashPair` types)
