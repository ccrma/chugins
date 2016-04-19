// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust string => dac;
// evaluate faust code
string.compile("string.dsp");

// time loop
while( true )
{
    // set
    string.v("gate",0);
    10::ms => now;
    string.v("gate",1);
    string.v("freq", Math.random2f(80,800) );
    // advance time
    500::ms => now;
}
