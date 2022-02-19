DbLiCKLFO osc;

SinOsc sinosc; 
SqrOsc sqrosc;
TriOsc triosc;
SawOsc sawosc;

osc => dac;
.25 => osc.gain => sinosc.gain => sqrosc.gain => triosc.gain => sawosc.gain;
200 => osc.freq => sinosc.freq => sqrosc.freq => triosc.freq => sawosc.freq;

if(0)
{
    <<<"DbSaw">>>;
    osc.saw();
    1::second => now;

    <<<"ChucKSaw">>>;
    osc.op(0); // disable
    sawosc => dac;
    1::second => now;
    sawosc =< dac;
    osc.op(1); // enable
}

if(0)
{
    <<<"DbSin">>>;
    osc.sine();
    1::second => now;

    <<<"ChucKSin">>>;
    osc.op(0); // disable
    sinosc => dac;
    1::second => now;
    sinosc =< dac;
    osc.op(1); // enabled
}

if(0)
{
    <<<"Sqr">>>;
    osc.sqr();
    1::second => now;

    <<<"ChucKSqr">>>;
    osc.op(0); // disable
    sqrosc => dac;
    1::second => now;
    sqrosc =< dac;
    osc.op(1); // enabled
}

if(0)
{
    <<<"Tri">>>;
    osc.tri();
    1::second => now;

    <<<"ChucKTri">>>;
    osc.op(0); // disable
    triosc => dac;
    1::second => now;
    triosc =< dac;
    osc.op(1); // enabled
}

if(0)
{
    <<<"Hyper">>>;
    osc.hyper();
    1::second => now;
}

if(0)
{
    <<<"Sample and hold">>>;
    Noise n => osc;
    osc.sampleHold();
    1::second => now;
}

if(0)
{
    <<<"Smooth SampleHold">>>;
    osc.smoothSampleHold();
    1::second => now;
}

if(1)
{
    osc.fnoise();
    1::second => now;
}

