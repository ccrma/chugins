// Eric Heep
// April 3rd, 2017
// power-adsr-test.ck

// unittests to help smooth out some kinks

class PowerADSRTest {

    dur a;
    dur d;
    dur r;
    float s;

    float ac, dc, rc;
    string errorMessages[0];

    int totalTests, passedTests;

    fun void setTimes(dur _a, dur _d, float _s, dur _r) {
        _a => a;
        _d => d;
        _s => s;
        _r => r;
    }

    fun void setCurves(float _ac, float _dc, float _rc) {
        _ac => ac;
        _dc => dc;
        _rc => rc;
    }

    fun void testEnvelopes() {
        // tests accuracy of state changes
        testAttackState();
        testDecayState();
        testSustainState();
        testReleaseState();
        testDoneState();

        // tests to see if the values between states
        // fall within an acceptable threshold
        testADValue();
        testDRValue();

        // tests that state changes happen
        // at precisely the correct time
        testALength();
        testADLength();

        // tests partial envelopes
        testPartialADValue();
        testPartialDRValue();
        testPartialRAValue();
    }

    fun void testGetters() {
        // sets and gets durations
        testGetAttackTime();
        testGetDecayTime();
        testGetReleaseTime();

        // sets and gets sustain
        testGetSustainLevel();

        // sets and gets curves
        testGetAttackCurve();
        testGetDecayCurve();
        testGetReleaseCurve();
    }

    fun void testAttackState() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();

        p =< blackhole;
        assert(p.state() == 1, "testAttackState");
    }

    fun void testDecayState() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a => now;

        p =< blackhole;
        assert(p.state() == 2, "testDecayState");
    }

    fun void testSustainState() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a + d => now;

        p =< blackhole;
        assert(p.state() == 3, "testSustainState");
    }

    fun void testReleaseState() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a + d => now;
        p.keyOff();

        p =< blackhole;
        assert(p.state() == 4, "testReleaseState");
    }

    fun void testDoneState() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a + d => now;
        p.keyOff();
        r => now;

        p =< blackhole;
        assert(p.state() == 0, "testDoneState");
    }

    fun void testADValue() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a - 1::samp => now;
        p.value() => float lastAttackVal;
        1::samp => now;
        p.value() => float firstDecayVal;

        p =< blackhole;
        assertLess(Math.fabs(lastAttackVal - firstDecayVal), 0.001, "testADValue");
    }

    fun void testDRValue() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        (a + d) - 1::samp => now;
        p.value() => float lastDecayVal;
        p.keyOff();
        1::samp => now;
        p.value() => float firstReleaseVal;

        p =< blackhole;
        assertLess(Math.fabs(lastDecayVal - firstReleaseVal), 0.001, "testDRValue");
    }

    fun void testALength() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        now => time begin;
        while (p.state() != 2) {
            1::samp => now;
        }

        p =< blackhole;
        assert(now - begin == a, "testALength");
    }

    fun void testADLength() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        now => time begin;
        while (p.state() != 3) {
            1::samp => now;
        }

        p =< blackhole;
        assert((now - begin) == (a + d), "testADLength");
    }

    fun void testPartialADValue() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        (a/2.0) - 1::samp => now;
        p.value() => float lastAttackValue;

        p.keyOff();
        1::samp => now;
        p.value() => float firstDecayValue;

        p =< blackhole;
        assertLess(Math.fabs(lastAttackValue - firstDecayValue), 0.001, "testPartialADValue");
    }

    fun void testPartialDRValue() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a => now;
        d/2.0 => now;
        p.value() => float lastDecayValue;
        p.keyOff();
        1::samp => now;
        p.value() => float firstReleaseValue;

        p =< blackhole;
        assertLess(Math.fabs(lastDecayValue - firstReleaseValue), 0.001, "testPartialDRValue");
    }

    fun void testPartialRAValue() {
        PowerADSR p => blackhole;
        p.set(a, d, s, r);
        p.setCurves(ac, dc, rc);
        p.keyOn();
        a => now;
        d => now;
        p.keyOff();
        r/2.0 => now;
        p.value() => float lastReleaseValue;
        1::samp => now;
        p.keyOn();
        1::samp => now;
        p.value() => float firstAttackValue;

        p =< blackhole;
        assertLess(Math.fabs(lastReleaseValue - firstAttackValue), 0.015, "testPartialRAValue");
    }

    fun void testGetAttackTime() {
        PowerADSR p => blackhole;
        Math.random2(2, 9)::second => dur attack;
        p.attackTime(attack);
        assert(p.attackTime() == attack, "testGetAttackTime");
        p =< blackhole;
    }

    fun void testGetDecayTime() {
        PowerADSR p => blackhole;
        Math.random2(2, 9)::second => dur decay;
        p.decayTime(decay);
        assert(p.decayTime() == decay, "testGetDecayTime");
        p =< blackhole;
    }

    fun void testGetSustainLevel() {
        PowerADSR p => blackhole;
        Math.random2f(0.1, 1.0) => float sustain;
        p.sustainLevel(sustain);
        assertLess(Math.fabs(p.sustainLevel() - sustain), 0.001, "testGetSustainLevel");
        p =< blackhole;
    }

    fun void testGetReleaseTime() {
        PowerADSR p => blackhole;
        Math.random2(2, 9)::second => dur release;
        p.releaseTime(release);
        assert(p.releaseTime() == release, "testGetReleaseTime");
        p =< blackhole;
    }

    fun void testGetAttackCurve() {
        PowerADSR p => blackhole;
        Math.random2f(0.5, 2.5) => float attackCurve;
        p.attackCurve(attackCurve);
        assertLess(Math.fabs(p.attackCurve() - attackCurve), 0.001, "testGetAttackCurve");
        p =< blackhole;
    }

    fun void testGetDecayCurve() {
        PowerADSR p => blackhole;
        Math.random2f(0.5, 2.5) => float decayCurve;
        p.decayCurve(decayCurve);
        assertLess(Math.fabs(p.decayCurve() - decayCurve), 0.001, "testGetDecayCurve");
        p =< blackhole;
    }

    fun void testGetReleaseCurve() {
        PowerADSR p => blackhole;
        Math.random2f(0.5, 2.5) => float releaseCurve;
        p.releaseCurve(releaseCurve);
        assertLess(Math.fabs(p.releaseCurve() - releaseCurve), 0.001, "testGetReleaseCurve");
        p =< blackhole;
    }

    fun string results() {
        chout <= IO.newline();
        <<< passedTests + "/" + totalTests + " PASSED", "" >>>;
        if (errorMessages.size() > 0) {
            for (0 => int i; i < errorMessages.size(); i++) {
                <<< errorMessages[i], "" >>>;
            }
        }
    }

    fun void update(int bool) {
        totalTests++;
        if (bool) {
            passedTests++;
        }
    }

    fun void logError(string error, string fnName) {
        errorMessages << fnName + " | " + error;
    }

    fun void assertLess(float a, float b, string fn) {
        if (a < b) {
            chout <= ".";
        } else {
            chout <= "E";
            logError(a + " !< " + b, fn);
        }
        chout.flush();
        update(a < b);
    }

    fun void assert(int bool, string fn) {
        if (bool) {
            chout <= ".";
        } else {
            cherr <= "E";
            logError("Does not assert true.", fn);
        }
        chout.flush();

        update(bool);
    }
}

PowerADSRTest p;

100::ms => dur attack;
100::ms => dur decay;
100::ms => dur release;

p.setTimes(attack, decay, 0.5, release);
p.setCurves(1.0, 1.0, 1.0);
p.testEnvelopes();

p.setTimes(attack, decay, 0.5, release);
p.setCurves(0.5, 0.5, 0.5);
p.testEnvelopes();

p.setTimes(attack, decay, 0.5, release);
p.setCurves(2.0, 2.0, 2.0);
p.testEnvelopes();

p.setTimes(attack, decay, 0.5, release);
p.setCurves(0.5, 2.0, 1.0);
p.testEnvelopes();

p.testGetters();

p.results();
