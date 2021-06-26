me.dir() + "plugins/TAL-NoiseMaker-64.dll" => string vst_effect_path; // Must be absolute path.

<<< "Trying VST at path: ", vst_effect_path >>>;

VST vst => dac;

if (vst.loadPlugin(vst_effect_path)) {
   <<< "success!" >>>;
   <<< "num parameters: ", vst.getNumParameters() >>>;
   <<< "parameter 0 is named: ", vst.getParameterName(0) >>>;
   // vst.loadPreset("presets/serum_preset.fxp");  // This preset file only works with the Serum VST.
   vst.setParameter(0, vst.getParameter(0));  // setting a parameter to its current value.
} else {
    <<< "fail!" >>>;
}

while(true) {
    vst.noteOn(60, 1.);
    200::ms => now;
    vst.noteOff(60, 0.);
    250::ms => now;
}
