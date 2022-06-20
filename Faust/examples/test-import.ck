// name: test-import.ck
// desc: Demonstrate that FaucK can import a .lib file

// You should hear a stereo sine tone at 440 Hz.

// instantiate and connect faust => ck
Faust fck;

me.dir() + "my_faust_code" => string librariesDir => fck.librariesDir;

<<<"librariesDir: ", librariesDir>>>;

// evaluate Faust synth
fck.eval(`
	import("my_library.lib");
	process = my_signal;  // defined in my_library.lib
`);

fck => dac;

// time loop
while( true )
{
	// advance time
	100::ms => now;
}
