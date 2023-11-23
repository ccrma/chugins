class LineTest extends Assert {
    0.0001 => float FP_MARGIN;
    {
        true => exitOnFailure;

        testSingleDefault();
        testSingleDuration();
        testSingleDurationAndTarget();
        testSingleWithInitial();
        testMulti();
        testMultiInit();
        testFailDifferentSizesSet();
        testFailDifferentSizesKeyOn();

        testKeyOnSingleDuration();
        testKeyOnSingleDurationAndTarget();
        testKeyOnSingleWithInitial();
        testKeyOnMulti();
        testKeyOnMultiInit();


        testKeyOnEvent();

        testKeyOff();
        testKeyOffDur();
        testKeyOffTarget();
        testKeyOffDurTarget();

        testKeyOffDurLen();

        testTargets();

        chout <= "success!" <= IO.newline();
    }

    public void testSingleDefault() {
           <<< "testSingleDefault" >>>;
           Step s => Line l => blackhole;

           l.keyOn();
           assertRamp(l, 1000::samp, 1);
    }

    public void testSingleDuration() {
           <<< "testSingleDuration" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           samps::samp => l.set;

           l.keyOn();
           assertRamp(l, samps::samp, 1);
    }

    public void testSingleDurationAndTarget() {
           <<< "testSingleDurationAndTarget" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           l.set(samps, samps::samp);

           l.keyOn();
           assertRamp(l, samps::samp, samps);
    }

    public void testSingleWithInitial() {
        <<< "testSingleWithInitial" >>>;
        Step s => Line l => blackhole;

        l.set(-1000, 1000, 1::second);

        l.keyOn();
        assertRamp(l, 1::second, -1000.0, 1000.0);
    }

    public void testMulti() {
           <<< "testMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [20::ms, 2::samp, 3::second] @=> dur durs[];

           l.set(targets, durs);
           l.keyOn();

           assertRampMulti(l, 0, targets, durs);
    }

    public void testMultiInit() {
           <<< "testMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [2::samp, 20::ms, 3::second] @=> dur durs[];

           l.set(init, targets, durs);
           l.keyOn();

           assertRampMulti(l, init, targets, durs);
    }

    public void testKeyOnSingleDuration() {
           <<< "testKeyOnSingleDuration" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           samps::samp => l.keyOn;

           assertRamp(l, samps::samp, 1);
    }

    public void testKeyOnSingleDurationAndTarget() {
           <<< "testKeyOnSingleDurationAndTarget" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           l.keyOn(samps, samps::samp);

           assertRamp(l, samps::samp, samps);
    }

    public void testKeyOnSingleWithInitial() {
        <<< "testKeyOnSingleWithInitial" >>>;
        Step s => Line l => blackhole;

        l.keyOn(-1000, 1000, 1::second);

        assertRamp(l, second, -1000.0, 1000.0);
    }

    public void testKeyOnMulti() {
           <<< "testKeyOnMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [2::samp, 20::ms, 3::second] @=> dur durs[];

           l.keyOn(targets, durs);

           assertRampMulti(l, 0, targets, durs);
    }

    public void testKeyOnMultiInit() {
           <<< "testKeyOnMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [2::samp, 20::ms, 3::second] @=> dur durs[];

           l.keyOn(init, targets, durs);

           assertRampMulti(l, init, targets, durs);
    }

    public void testKeyOnEvent() {
           <<< "testKeyOnEvent" >>>;
           Step s => Line l => blackhole;

           l.set(1::second);

           now => time prev;

           l.keyOn() => now;

           assertEquals(1::second, now - prev);
    }

    public void testKeyOffDurLen() {
           <<< "testKeyOffDur" >>>;
           Step s => Line l => blackhole;

           1::second => dur want;

           l.keyOff(want) => dur got;

           assertEquals(want, got);
    }

    public void testKeyOff() {
           <<< "testKeyOff" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff();
           assertRamp(l, 1000::samp, 0.5, 0);
    }

    public void testKeyOffDur() {
           <<< "testKeyOffDur" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(1::second);
           assertRamp(l, 1::second, -0.5, -1);
    }

    public void testKeyOffTarget() {
           <<< "testKeyOffTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15);
           assertRamp(l, 1000::samp, -0.5, 15);
    }

    public void testKeyOffDurTarget() {
           <<< "testKeyOffDurTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15, 1::second);
           assertRamp(l, 1::second, -0.5, 15);
    }

    public void testFailDifferentSizesSet() {
           <<< "testFailDifferentSizesSet" >>>;
           Step s => Line l => blackhole;

           l.set([0.0, 1, 2], [1::second]) => int got;

           assertFalse(got);
    }

    public void testFailDifferentSizesKeyOn() {
           <<< "testFailDifferentSizesKeyOn" >>>;
           Step s => Line l => blackhole;

           l.keyOn([0.0, 1, 2], [1::second]) @=> dur got;
           0::samp => dur want;

           assertEquals(want, got);
    }
    
    public void testTargets() {
       <<< "testTargets" >>>;
       Step s => Line l => blackhole;

       [1.0, -2, -3] @=> float targets[];
       [2::samp, 2::ms, 3::ms] @=> dur durs[];

       l.set(targets, durs);

       l.targets() @=> float got[];

       assertEquals(got.size(), targets.size());

       for (int i: Std.range(got.size())) {
              assertEquals(got[i], targets[i], FP_MARGIN);
       }
    }

    public void testDurs() {
       <<< "testDurs" >>>;
       Step s => Line l => blackhole;

       [1.0, -2, -3] @=> float targets[];
       [2::samp, 2::ms, 3::ms] @=> dur durs[];

       l.set(targets, durs);

       l.durations() @=> dur got[];

       assertEquals(got.size(), durs.size());

       for (int i: Std.range(got.size())) {
              assertEquals(got[i], durs[i]);
       }
    }

    fun void assertRamp(Line @ l, dur d, float target) {
        assertRamp(l, d, 0, target);
    }

    fun void assertRamp(Line @ l, dur d, float init, float target) {
        (d / samp) $ int => int samps;
        // validate ramp
        for(int i: Std.range(samps)) {
                (target - init) * ((i $ float) / samps) + init => float want;
                assertEquals(want, l.last(), FP_MARGIN);
                samp => now;
        }

        // validate that clipping after ramp works
        for(int i: Std.range(1000)) {
                assertEquals(target, l.last(), FP_MARGIN);
                samp => now;
        }
    }

    fun void assertRampMulti(Line @ l, float initial, float targets[], dur durs[]) {
        initial => float prev;
        for (int i: Std.range(targets.size())) {
            (durs[i] / samp) $ int => int samps;
            targets[i] => float target;
            // validate ramp
            for(int j: Std.range(samps)) {
                (target - prev) * ((j $ float) / samps) + prev => float want;
                assertEquals(want, l.last(), FP_MARGIN);
                samp => now;
            }
            target => prev;
        }

        // validate that clipping after ramp works
        for(int i: Std.range(1000)) {
                assertEquals(targets[-1], l.last(), FP_MARGIN);
                samp => now;
        }
    }
}

LineTest lineTest;
1::samp => now;
