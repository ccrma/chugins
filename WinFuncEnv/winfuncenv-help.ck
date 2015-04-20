// WinFunEnv is a AR envelope built around window functions! 

// ~options
//
// set (duration, duration), default (0.0, 0.0)
//   sets duration for attack and release phase
//
// attack (duration), default 0.0
//   sets length for the attack phase
//
// release (duration), default 0.0
//   sets length for the release phase
// 
// ~ the following window functions are available to use as envelopes
// ~ certain functions allow for custom coefficient values (ie setBlackman(0.64) )
//
// setBlackman()
// setBlackman(float a)
// setBlackmanHarris()
// setBlackmanDerivative (float a0, float a1, float a2, float a3);
// setBlackmanNutall()
// setExponential()
// setExponential(a)
// setHann()
// setHannPoisson()
// setHannPoisson(a)
// setNutall()
// setParzen()
// setPoisson()
// setPoisson(float a)
// setTukey()
// setTukey(float a)
// setWelch
//
// ~ hopefully further updates will see more windows being added (Kaiser, etc)

Noise nois  => WinFuncEnv win1 => dac;

fun void playWindow(WinFuncEnv @ win, dur env_time) {
    win.attack(env_time);
    win.release(env_time);
    win.keyOn(); 
    env_time => now;
    win.keyOff(); 
    env_time => now;
}

// sets Blackman Envelope with a default value of 0.16
win1.setBlackman();
<<< "Blackman Envelope with default value of 0.16 on white noise", "" >>>;
playWindow(win1, 2.5::second);

// sets Blackman Envelope with a custom value
win1.setBlackman(Math.random2f(0.05, 0.25));
<<< "Blackman Envelope with a random custom value on white noise.", "" >>>;
playWindow(win1, 2.5::second);

// sets BlackmanHarris Envelope
win1.setBlackmanHarris();
<<< "BlackmanHarris Envelope on white noise.", "" >>>;
playWindow(win1, 2.5::second);

// replacing noise with a sine wave
nois =< win1;
SinOsc sin => win1;

// sets Parzen Envelope
win1.setParzen();
<<< "Parzen Window used for amplitude modulation on a sine wave", "" >>>;
for (int i; i < 25000; i++) {
    playWindow(win1, 0.1::ms);
}

// sets Hann Envelope
win1.setHann();
<<< "Hann Window used for amplitude modulation on a sine wave", "" >>>;
for (int i; i < 25000; i++) {
    playWindow(win1, 0.1::ms);
}

// removing sine wave, adding noise and a second envelope to be in series with the first
sin =< win1;
nois => win1 =< dac;
win1 => WinFuncEnv win2 => dac;

<<< "Two envelopes, one Parzen and one Welch in series with each other with slightly different envelope times.", "" >>>;
win1.setParzen();
win2.setWelch();

fun void loop(WinFuncEnv @ win, dur env_time) {
    repeat(500) {
        playWindow(win, env_time);
    }
}

spork ~ loop(win1, 70::ms);
loop(win2, 73::ms);
