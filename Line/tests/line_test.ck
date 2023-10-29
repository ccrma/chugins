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

        testKeyOnEvent();

        testKeyOff();
        testKeyOffDur();
        testKeyOffTarget();
        testKeyOffDurTarget();

        testKeyOffEvent();

        chout <= "success!" <= IO.newline();
    }

    public void testSingleDefault() {
           <<< "testSingleDefault" >>>;
           Step s => Line l => blackhole;

           assertKeyOn(l, 1000, 1);
    }

    public void testSingleDuration() {
           <<< "testSingleDuration" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           samps::samp => l.set;

           assertKeyOn(l, samps, 1);
    }

    public void testSingleDurationAndTarget() {
           <<< "testSingleDurationAndTarget" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           l.set(samps, samps::samp);

           assertKeyOn(l, samps, samps);
    }

    public void testSingleWithInitial() {
        <<< "testSingleWithInitial" >>>;
        Step s => Line l => blackhole;

        l.set(-1000, 1000, 1::second);

        assertKeyOn(l, (second / samp) $ int, -1000.0, 1000.0);
    }

    public void testMulti() {
           <<< "testMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [2::samp, 2::ms, 3::ms] @=> dur durs[];

           l.set(targets, durs);

           assertKeyOnMulti(l, 0, targets, durs);
    }

    public void testMultiInit() {
           <<< "testMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [2::samp, 2::ms, 3::ms] @=> dur durs[];

           l.set(init, targets, durs);

           assertKeyOnMulti(l, init, targets, durs);
    }

    public void testKeyOnEvent() {
           <<< "testKeyOnEvent" >>>;
           Step s => Line l => blackhole;

           l.set(1::second);

           now => time prev;

           l.keyOn() => now;

           assertEquals(1::second, now - prev);
    }

    public void testKeyOffEvent() {
           <<< "testKeyOffEvent" >>>;
           Step s => Line l => blackhole;

           now => time prev;

           l.keyOff(1::second) => now;

           assertEquals(1::second, now - prev);
    }

    public void testKeyOff() {
           <<< "testKeyOff" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff();
           assertKeyOff(l, 1000::samp, 0.5, 0);
    }

    public void testKeyOffDur() {
           <<< "testKeyOffDur" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(1::second);
           assertKeyOff(l, 1::second, -0.5, -1);
    }

    public void testKeyOffTarget() {
           <<< "testKeyOffTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15);
           assertKeyOff(l, 1000::samp, -0.5, 15);
    }

    public void testKeyOffDurTarget() {
           <<< "testKeyOffDurTarget" >>>;

           Step s => Line l => blackhole;

           // go for half of the keyon...
           l.set(-1, 0, 1::second);

           l.keyOn();
           0.5::second => now; // stop ramp early

           l.keyOff(15, 1::second);
           assertKeyOff(l, 1::second, -0.5, 15);
    }

    fun void assertKeyOn(Line @ l, int samps, float target) {
        assertKeyOn(l, samps, 0, target);
    }

    fun void assertKeyOn(Line @ l, int samps, float init, float target) {
        l.keyOn();

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

    fun void assertKeyOnMulti(Line @ l, float initial, float targets[], dur durs[]) {
        l.keyOn();

        initial => float prev;
        for (int i: Std.range(targets.size())) {
            (durs[i] / samp) $ int => int samps;
            targets[i] => float target;
            // validate ramp
            for(int i: Std.range(samps)) {
                (target - prev) * ((i $ float) / samps) + prev => float want;
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

    fun void assertKeyOff(Line @ l, dur d, float init, float target) {
        l.keyOff();

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
}

LineTest lineTest;
1::samp => now;
