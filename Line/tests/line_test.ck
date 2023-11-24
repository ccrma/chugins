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
        
        testKeyOnInit();
        testKeyOnSingleDuration();
        testKeyOnSingleDurationAndTarget();
        testKeyOnSingleWithInitial();
        testKeyOnMulti();
        testKeyOnMultiInit();

        testMultipleKeyOn();


        testKeyOnEvent();

        testKeyOff();
        testKeyOffDur();
        testKeyOffTarget();
        testKeyOffDurTarget();

        testKeyOffDurLen();

        // constructor tests
        testConstructorSingleDuration();
        testConstructorSingleDurationAndTarget();
        testConstructorMulti();
        testConstructorMultiInit();

        testGetDurations();
        testGetTargets();

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

    public void testConstructorSingleDuration() {
           <<< "testConstructorSingleDuration" >>>;

           11111::samp => dur samps;
           Step s => Line l(samps) => blackhole;

           l.keyOn();
           assertRamp(l, samps, 1);
    }

    public void testSingleDurationAndTarget() {
           <<< "testSingleDurationAndTarget" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           l.set(samps, samps::samp);

           l.keyOn();
           assertRamp(l, samps::samp, samps);
    }

    public void testConstructorSingleDurationAndTarget() {
           <<< "testConstructorSingleDurationAndTarget" >>>;

           11111::samp => dur samps;
           11111 => float target;
           Step s => Line l(target, samps) => blackhole;

           l.keyOn();
           assertRamp(l, samps, target);
    }

    public void testSingleWithInitial() {
        <<< "testSingleWithInitial" >>>;
        Step s => Line l => blackhole;

        l.set(-1000, 1000, 1::second);

        l.keyOn();
        assertRamp(l, 1::second, -1000.0, 1000.0, true);
    }

    public void testMulti() {
           <<< "testMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           l.set(targets, durs);
           l.keyOn();

           assertSamps(l, [0.0, 0.666666, 0, -2, -2.6666666, -3, -3]);
    }

    public void testConstructorMulti() {
           <<< "testConstructorMulti" >>>;

           [1.0, -2, -3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           Step s => Line l(targets, durs) => blackhole;

           l.keyOn();

           assertSamps(l, [0.0, 0.666666, 0, -2, -2.6666666, -3, -3]);
    }

    public void testMultiInit() {
           <<< "testMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           l.set(init, targets, durs);
           l.keyOn();

           assertSamps(l, [-1.0, 0.33333, 0, -2, 1.3333, 3, 3]);
    }

    public void testConstructorMultiInit() {
           <<< "testConstructorMultiInit" >>>;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           Step s => Line l(init, targets, durs) => blackhole;

           l.keyOn();

           assertSamps(l, [-1.0, 0.33333, 0, -2, 1.3333, 3, 3]);
    }

    public void testKeyOnInit() {
           <<< "testKeyOnInit" >>>;
           Step s => Line l(11111::samp) => blackhole;

           l.keyOn(0.5);

           assertRamp(l, 11111::samp, 0.5, 1, true);
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

        assertRamp(l, second, -1000.0, 1000.0, true);
    }

    public void testKeyOnMulti() {
           <<< "testKeyOnMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           l.keyOn(targets, durs);

           assertSamps(l, [0.0, 0.666666, 0, -2, -2.6666666, -3, -3]);
    }

    public void testKeyOnMultiInit() {
           <<< "testKeyOnMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [1.5::samp, 1.5::samp, 1.5::samp] @=> dur durs[];

           l.keyOn(init, targets, durs);

           assertSamps(l, [-1.0, 0.33333, 0, -2, 1.3333, 3, 3]);
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
           assertRamp(l, 1000::samp, 0.5, 0, 247);
    }

    public void testKeyOffDur() {
           <<< "testKeyOffDur" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(1::second);
           assertRamp(l, 1::second, -0.5, -1, true);
    }

    public void testKeyOffTarget() {
           <<< "testKeyOffTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15);
           assertRamp(l, 1000::samp, -0.5, 15, true);
    }

    public void testKeyOffDurTarget() {
           <<< "testKeyOffDurTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15, 1::second);
           assertRamp(l, 1::second, -0.5, 15, true);
    }

    public testMultipleKeyOn() {
           <<< "testMultipleKeyOn" >>>;
           11111::samp => dur samps;
           Step s => Line l(samps) => blackhole;

           l.keyOn();
           assertRamp(l, samps, 0, 1, false);
           l.keyOn(0, samps);
           assertRamp(l, samps, 1, 0, false);
           l.keyOn(1, samps);
           assertRamp(l, samps, 0, 1, false);
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

    public testGetDurations() {
        <<< "testGetDurations" >>>;

        [1::ms, 2::ms, 3::ms, 4::ms] @=> dur want[];
        Step s => Line l([1.0,2,3,4], want) => blackhole;

        l.durations() @=> dur got[];

        assertEquals(want.size(), got.size());

        for (int i: Std.range(want.size())) {
            assertEquals(want[i], got[i]);
        }

    }

    public testGetTargets() {
        <<< "testGetTargets" >>>;

        [1.0,2,3,4] @=> float want[];
        Step s => Line l(want,  [1::ms, 2::ms, 3::ms, 4::ms]) => blackhole;

        l.targets() @=> float got[];

        assertEquals(want.size(), got.size());

        for (int i: Std.range(want.size())) {
            assertEquals(want[i], got[i], FP_MARGIN);
        }

    }

    fun void assertRamp(Line @ l, dur d, float target) {
        assertRamp(l, d, 0, target, true);
    }

    fun void assertRamp(Line @ l, dur d, float init, float target, int clipping) {
        (d / samp) $ int => int samps;
        // validate ramp
        for(int i: Std.range(samps)) {
                (target - init) * ((i $ float) / samps) + init => float want;
                // <<< want, l.last() >>>;
                assertEquals(want, l.last(), FP_MARGIN);
                samp => now;
        }

        if (!clipping) return;

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

            // because durations can be sub-sample, need to properly account for this.
            durs[i] / samp => float sampsF;
            targets[i] => float target;
            // validate ramp
            for(int i: Std.range(samps)) {
                (target - prev) * ((i $ float) / sampsF) + prev => float want;
                // <<< want, l.last() >>>;
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

    fun void assertSamps(Line @ l, float wants[]) {
        for (auto want: wants) {
            // <<< want, l.last() >>>;
            assertEquals(want, l.last(), FP_MARGIN);
            samp => now;
        }
    }
}

LineTest lineTest;
1::samp => now;
