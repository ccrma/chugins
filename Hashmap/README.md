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

// clear 
hm.clear()
```

## Details

The underlying implementation is provided by [hashmap.c](https://github.com/tidwall/hashmap.c). It uses open addressing (i.e. probing a contiguous array for collision resolution rather than chaining.) As such it is cache friendly and iteration is relatively fast.

Hashmap currently uses [xxHash](https://github.com/Cyan4973/xxHash) as its hashing function. Perhaps in the future we can add others, or the option for users to supply their own.

Hashmap uses a grow-factor of 0.60, and a shrink-factor of 0.10.

Objects are hashed by reference (using their pointer address). Two different objects with the same values will NOT resolve to the same hash. 

See `hashmap-test.ck` for (relatively) comprehensive unit tests. Refcounting is accounted for, at least for the current test coverage.

## Known Bugs

If hashmaps form a cyclic dependency (e.g. 2 hashmaps point to each other) the json functions `HashMap.toJson()` will crash.

## Benchmarks

`hm_bench.cpp` contains benchmark hashmap tests. Performance is similar or better than `std::unordered_map`

## Next Steps?

- Support other hash functions
- User-defined hash functions
- user-defined grow / shrink factors
- If/when ChucK adds generics Hashmap will either become obsolete or need to be reworked
- add an iterator (requires adding `HashItem` and `HashPair` types)
