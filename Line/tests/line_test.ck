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

        chout <= "success!" <= IO.newline();
    }

    public void testSingleDefault() {
           <<< "testSingleDefault" >>>;
           Step s => Line l => blackhole;

           assertRamp(l, 1000, 1);
    }

    public void testSingleDuration() {
           <<< "testSingleDuration" >>>;    
           Step s => Line l => blackhole;

           11111 => int samps;
           samps::samp => l.set;

           assertRamp(l, samps, 1);
    }

    public void testSingleDurationAndTarget() {
           <<< "testSingleDurationAndTarget" >>>;
           Step s => Line l => blackhole;

           11111 => int samps;
           l.set(samps, samps::samp);

           assertRamp(l, samps, samps);
    }

    public void testSingleWithInitial() {
        <<< "testSingleWithInitial" >>>;
        Step s => Line l => blackhole;

        l.set(-1000, 1000, 1::second);

        assertRamp(l, (second / samp) $ int, -1000.0, 1000.0);
    }

    public void testMulti() {
           <<< "testMulti" >>>;
           Step s => Line l => blackhole;

           [1.0, -2, -3] @=> float targets[];
           [2::samp, 2::ms, 3::ms] @=> dur durs[];

           l.set(targets, durs);

           assertRampMulti(l, 0, targets, durs);
    }

    public void testMultiInit() {
           <<< "testMultiInit" >>>;
           Step s => Line l => blackhole;

           -1.0 => float init;
           [1.0, -2, 3] @=> float targets[];
           [2::samp, 2::ms, 3::ms] @=> dur durs[];

           l.set(init, targets, durs);

           assertRampMulti(l, init, targets, durs);
    }

    fun void assertRamp(Line @ l, int samps, float target) {
        assertRamp(l, samps, 0, target);
    }

    fun void assertRamp(Line @ l, int samps, float init, float target) {
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

    fun void assertRampMulti(Line @ l, float initial, float targets[], dur durs[]) {
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
}

LineTest lineTest;
1::samp => now;
