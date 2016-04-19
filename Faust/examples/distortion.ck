// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust distortion => dac;
// compile faust code
distortion.eval("import(\"effect.lib\"); process = cubicnl_demo ;");
// parameter dump
distortion.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
