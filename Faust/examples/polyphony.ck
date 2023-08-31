// name: polyphony.ck
// desc: demo of Faust chugin in action!

// device to open (see: chuck --probe)
1 => int device;
// get from command line
if( me.args() ) me.arg(0) => Std.atoi => device;

Hid hi;
HidMsg msg;

// try to open MIDI port (see chuck --probe for available devices)
if( !hi.openKeyboard( device ) ) me.exit();

// make our own event
class NoteEvent extends Event
{
    int note;
    int velocity;
}

// the event
NoteEvent on;
// array of ugen's handling each note
Event @ us[128];

// handler for a single voice
fun void handler()
{
    // don't connect to dac until we need it
    Faust fck;
    fck.eval(`
      process = pm.elecGuitar_ui_MIDI <: _,_;
    `);
    Event off;
    int note;

    while( true )
    {
        on => now;
        on.note => note;
        // dynamically repatch
        fck => dac;
        fck.v("/elecGuitar/midi/freq",Std.mtof( note ));
        fck.v("/elecGuitar/midi/gain",( on.velocity / 128.0 ));
        fck.v("/elecGuitar/gate",1);
        off @=> us[note];

        off => now;
        fck.v("/elecGuitar/gate",0);
        null @=> us[note];
        fck =< dac;
    }
}

// spork handlers, one for each voice
for( 0 => int i; i < 20; i++ ) spork ~ handler();

fun int keybToMidi(int keybKey){
  60 => int midiNote;
  if(keybKey == 44){
    60 => midiNote;
  }
  else if(keybKey == 31){
    61 => midiNote;
  }
  else if(keybKey == 45){
    62 => midiNote;
  }
  else if(keybKey == 32){
    63 => midiNote;
  }
  else if(keybKey == 46){
    64 => midiNote;
  }
  else if(keybKey == 47){
    65 => midiNote;
  }
  else if(keybKey == 34){
    66 => midiNote;
  }
  else if(keybKey == 48){
    67 => midiNote;
  }
  else if(keybKey == 35){
    68 => midiNote;
  }
  else if(keybKey == 49){
    69 => midiNote;
  }
  else if(keybKey == 36){
    70 => midiNote;
  }
  else if(keybKey == 50){
    71 => midiNote;
  }
  else if(keybKey == 51){
    72 => midiNote;
  }
  return midiNote;
}

// infinite time-loop
while( true )
{
    // wait on midi event
    hi => now;

    // get the midimsg
    while( hi.recv( msg ) )
    {
        keybToMidi(msg.which) => int currentNote;
        // catch only noteon
        if( msg.isButtonDown() ){
          // store midi note number
          currentNote => on.note;
          // store velocity
          100 => on.velocity;
          // signal the event
          on.signal();
          // yield without advancing time to allow shred to run
          me.yield();
        }
        else {
          us[currentNote].signal();
        }
    }
}
