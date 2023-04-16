declare version "0.0";
declare author "JOS, revised by RM, revised by David Braun";
declare description "Compressor demo application";

import("stdfaust.lib");

compressor_demo(aux_L, aux_R, main_L, main_R) = result
with{
	comp_group(x) = hgroup("COMPRESSOR [tooltip: Reference:
		http://en.wikipedia.org/wiki/Dynamic_range_compression]", x);

	settings_group(x)  = comp_group(hgroup("[0] Settings", x));
	meter_group(x)	= comp_group(hgroup("[1] Meters", x));

    gain_view = _ <: attach(_,abs : ba.linear2db : meter_group(vbargraph("Level",-60,10)));

    cgm = gain_view(co.compression_gain_mono(ratio,threshold,attack,release,aux))
    with {
        aux = 0.5*(abs(aux_L)+abs(aux_R));
    };

    compressor_stereo_demo = main_L * cgm * makeupgain , main_R * cgm * makeupgain;

	ratio = settings_group(hslider("[0] Ratio [style:knob]
	[tooltip: A compression Ratio of N means that for each N dB increase in input
	signal level above Threshold, the output level goes up 1 dB]",
	5, 1, 20, 0.1));

	threshold = settings_group(hslider("[1] Threshold [unit:dB] [style:knob]
	[tooltip: When the signal level exceeds the Threshold (in dB), its level
	is compressed according to the Ratio]",
	-30, -100, 10, 0.1));

	attack = settings_group(hslider("[2] Attack [unit:ms] [style:knob] [scale:log]
	[tooltip: Time constant in ms (1/e smoothing time) for the compression gain
	to approach (exponentially) a new lower target level (the compression
	`kicking in')]", 50, 1, 1000, 0.1)) : *(0.001) : max(1/ma.SR);

	release = settings_group(hslider("[3] Release [unit:ms] [style: knob] [scale:log]
	[tooltip: Time constant in ms (1/e smoothing time) for the compression gain
	to approach (exponentially) a new higher target level (the compression
	'releasing')]", 500, 1, 1000, 0.1)) : *(0.001) : max(1/ma.SR);

	makeupgain = settings_group(vslider("[4] Makeup Gain [unit:dB]
	[tooltip: The compressed-signal output level is increased by this amount
	(in dB) to make up for the level lost due to compression]",
	0, -96, 96, 0.1)) : ba.db2linear;

    result = compressor_stereo_demo;
};

process = compressor_demo;