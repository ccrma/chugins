// name: flute.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust flute => dac;

// evaluate Faust code
flute.eval(`
  process = pm.flute_ui_MIDI <: _,_;
`);

// dump parameters
flute.dump();

// time loop
while( true )
{
  flute.v("/flute/gate",1); // start note
  flute.v("/flute/midi/freq", Math.random2f(100,800) ); // assign pitch
  300::ms => now; // "sustain"
  flute.v("/flute/gate",0); // end note
  100::ms => now; // give it some time to "breath"
}
