<<<"creating a KasFilter", "">>>;
BlitSaw s => KasFilter kf => dac;

110 => s.freq;
<<<"setting variables", "">>>;
.9 => kf.gain;
.7 => kf.resonance;
.3 => kf.accent;
<<<"testing sound using a sweep", "">>>;
for (110 => float x; x < 5000; 1.0003 *=>x)
{
	x => kf.freq;
	ms => now;
}
<<<"testing get functions", "">>>;
<<<"freq = ", kf.freq()>>>;
<<<"resonance = ", kf.resonance()>>>;
<<<"accent = ", kf.accent()>>>;
0 => s.freq;
0 => s.op;
2::second => now;
