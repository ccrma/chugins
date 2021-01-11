"C:/VSTPlugIns/Serum_x64.dll" => string vst_path;  // Must use absolute path.
<<< "Trying VST at path: ", vst_path >>>;

// The preset path can be absolute or relative to the working directory in which chuck.exe is called.
// The working directory isn't necessarily where chuck.exe is located.
"serum_preset.fxp" => string preset_path;  

VST vst => dac;

if (vst.loadPlugin(vst_path)) {
   <<< "success!" >>>;
   <<< "num parameters: ", vst.getNumParameters() >>>;
   <<< "parameter 0 is named: ", vst.getParameterName(0) >>>;
   vst.loadPreset(preset_path);
   vst.setParameter(0, vst.getParameter(0));
   // vst.setParameter(2, 1.0);
} else {
    <<< "fail!" >>>;
}

while( true )
{
    vst.noteOn(60, 1.);
    200::ms => now;

    vst.noteOff(60, 0.);
    250::ms => now;
}
