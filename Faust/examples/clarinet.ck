// name: clarinet.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust clarinet => dac;

// evaluate Faust code
clarinet.eval(`
  process = pm.clarinet_ui_MIDI <: _,_;
`);

// print the parameters of the Faust object
clarinet.dump();

// time loop
while( true )
{
  clarinet.v("/clarinet/gate",1); // start note
  clarinet.v("/clarinet/midi/freq", Math.random2f(100,800) ); // assign pitch
  300::ms => now; // "sustain"
  clarinet.v("/clarinet/gate",0); // end note
  100::ms => now; // give it some time to "breath"
}
