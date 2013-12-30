//////////////////////////////////////////////////////
// Spectacle - FFT-based spectral delay and EQ      //
//                                                  //
// SPECTACLE is an FFT-based delay instrument by    //
// John Gibson and inspired by the totally awesome  //
// Spektral Delay plug-in by Native Instruments.    //
// This version is adapted from RTcmix (rtcmix.org) //
// by Joel Matthys                                  //
//////////////////////////////////////////////////////

// Options
//
// clear (void) : reset Spectacle
// fftlen (int) : set FFT frame size (power of 2)
// overlap (int) : set frame overlap; best between 2 and 6
// delayMax (dur) : maximum delay time
// delayMin (dur) : minimum delay time
// freqMax (float) : maximum frequency processed by Spectacle
// freqMin (float) : minimum frequency processed by Spectacle
// range (float, float) : set both min and min freqs in one command
// bands (int) : set number of frequency bands, 1-512, default 128
// delay (dur) : set the same duration for all bands
// eq (float) : set the same EQ value for all bands (value is +/- dB)
// feedback (float) : set the same feedback value for all bands (-1.0 - 1.0)
//
// table (string, string) : set delay, eq, or feedback tables to
//                          random, ascending, or descending
//    example: table ("delay", "random");

// warning: use headphones or you'll get feedback!
adc => Spectacle spect => dac;
spect.range(0,5000);
<<< "Spectacle: random delay by default." >>>;
10::second => now;
<<< "Spectacle: switching to ascending delay and descending eq.">>>;
spect.table("delay","ascending");
spect.table("eq","descending");
10::second => now;
<<< "Spectacle: ascending eq, with feedback." >>>;
0 => spect.eq;
0.8 => spect.feedback;
20::second => now;
adc =< spect; // disconnect input
minute => now; // let it ring down