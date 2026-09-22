# Known Issues

## Patch Chugin Incompatibility (Segfault) — fixed in Patch

**Status:** Root cause is in the Patch chugin, not DrumBuf: Patch's
`disconnect()` leaves its method/vtable state set, so the next `tick()` calls
into a null API pointer (it also leaks one `Chuck_DL_Arg` per sample). A fix
for Patch is pending upstream; with it, `tests/test-patch-crash.ck` runs to
completion. Until it lands, the workaround below still applies, and
`tests/run-tests.sh` skips the two Patch tests unless `PATCH_CHUG` points at a
fixed build.

**Symptom (with an unfixed Patch):**
Segmentation fault occurs after using `Patch` to modulate DrumBuf parameters and then calling `disconnect()`.

**Example that crashes:**
```chuck
SinOsc lfo => Patch p(sampler, "transpose") => blackhole;
// ... play some notes ...
p.disconnect();  // Works fine
1::second => now;  // CRASHES here
```

**Workaround:**
Instead of using Patch for parameter modulation, use direct assignment:

```chuck
SinOsc lfo => blackhole;
while (true) {
    lfo.last() * 5.0 => sampler.transpose;  // Manual modulation
    1::samp => now;
}
```
