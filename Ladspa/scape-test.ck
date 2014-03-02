// test of Scape Chubgraph
//
// This example uses the caps library
// from http://quitte.de/dsp/caps.html
// installed in /usr/local/lib/ladspa/caps.so
// (If it's in another location, change the path
// in Scape.ck)
//
// Run with: chuck Scape.ck scape-test.ck

adc => Scape scape => dac;
scape.ladspa.info();
120 => scape.bpm;
minute => now;