
adc => Delay d => VSTHost vst => dac;

1::second => d.max => d.delay;

//"/Users/spencer/Library/Audio/Plug-Ins/VST/DistortionDC.vst" => vst.load;
"/Library/Audio/Plug-Ins/VST/FM8.vst" => vst.load;
vst.edit();

1::day => now;
