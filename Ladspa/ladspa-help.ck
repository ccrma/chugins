////////////////////////////////////////////////////////////////
// LADSPA plugin host                                         //
//                                                            //
// Load and run LADSPA plugins, incorporating them into the   //
// signal chain and changing settings on the fly.             //
//                                                            //
// version by Joel Matthys (joel matthysmusic dot com)        //
////////////////////////////////////////////////////////////////

// Options
// load (string): path to LADSPA file
// list (): list the available plugins in the LADSPA file
// info (): list the input and output ports for plugins
// activate (string): activate plugin based on label name
// set (int, float): set the nth parameter to a new value
// get (int): get the nth parameter. This could be either a control
//            input or output
// verbose (int): turn on/off notifications about plugin (default: 1)

"/usr/local/lib/ladspa/filter.so" => string plugname;
"lpf" => string labelname;
if (me.args()>0) me.arg(0) => plugname;
if (me.args()>1) me.arg(1) => labelname;

Noise n => LADSPA plugin => dac;
0.5 => n.gain;
// load requires either a full path or a relative path. If the
// LADSPA file is in the same directory, you need to prepend ./
// On OSX and Linux, plugins are usually in /usr/local/lib/ladspa
// or /usr/lib/ladspa.
plugname => plugin.load;

// Any individual LADSPA file may have several different plugins
// or versions of a plugin contained with it. For instance, the
// simple filter example that comes with the LADSPA SDK has two
// different plugins, a high-pass filter and a low-pass.
//
// list() allows you to see which plugins are available and what
// their labels are (which you need to know to activate them).
plugin.list();

// This activates the specific plugin. (Warning: there is a
// chance that this may freeze the VM for a moment.)
labelname => plugin.activate;

// This shows the input and output ports for the plugin. The
// audio ports are connected automatically. The control ports
// are numbered from 0.
plugin.info();
second => now;

// set parameter 0 to a value of 100. The console should show
// the name of the parameter that is being changed.
plugin.set(0,100);
second => now;
