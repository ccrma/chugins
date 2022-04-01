DbLiCKDistort distort;
SndBuf o => distort => dac;
o.read("../../PitchTrack/data/obama.wav");
o.loop(1);

<<< "Defaults are rather subtle.",
"Effects are best experimented with within Fiddle and a guitar.">>>;

["WaveShaper", "Atan", "Chew", "Clip", "Duff", 
 "Frostburn", "FullRectifier", "Offset", "Phase", "Invert",
 "KijjazDist", "KijjazDist2", "KijjazDist3", "KijjazDist4",
 "Ribbon", "Tanh"
 ] @=> string effects[];

for(int i; i<effects.size();i++)
{
    <<<i, effects[i]>>>;
    distort.distortion(effects[i]);
    2::second => now;
}