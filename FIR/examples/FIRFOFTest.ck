//  Using FIR filters to make formants
// by Perry R. Cook, October 2012
// This is generally a dumb idea, because 
// formants should be slowly time varying,
// but it gives another idea of the silly
// and amazing things we can do with filters.

Impulse glot => FIR fof1 => dac;
glot => FIR fof2 => dac;
glot => FIR fof3 => dac;

1024 => fof1.order => fof2.order => fof3.order;

[[300.0, 870.0, 2240.0],
 [730.0,1090.0,2440.0],
 [270.0,2290.0,3010.0]] @=> float vowels[][];

loadVowel(0);

while (1)  {
    0.1 => glot.next;
    Std.rand2f(0.01,0.05) :: second => now;
    if (Std.rand2(0,10) == 0) loadVowel(Std.rand2(0,2));
}

fun void loadVowel(int which) {
  for (0 => int i; i < 1024; i++)  {
    fof1.coeff(i,Math.sin(2.0*pi*i*vowels[which][0]/44100.0)*Math.exp(-i*0.01));
    fof2.coeff(i,Math.sin(2.0*pi*i*vowels[which][1]/44100.0)*Math.exp(-i*0.01));
    fof3.coeff(i,Math.sin(2.0*pi*i*vowels[which][2]/44100.0)*Math.exp(-i*0.01));
  }
}



