"C:/VSTPlugIns/ValhallaRoom_x64.dll" => string vst_effect_path; // Must be absolute path.

<<< "Trying VST at path: ", vst_effect_path >>>;

SndBuf b => Pan2 pan => VST vst =>dac;

-.5 => pan.pan; // test that the VST can take stereo input.

"special:dope" => b.read;

if (vst.loadPlugin(vst_effect_path)) {
   <<< "success!" >>>;
   <<< "num parameters: ", vst.getNumParameters() >>>;
   <<< "parameter 0 is named: ", vst.getParameterName(0) >>>;
   vst.setParameter(0, vst.getParameter(0));
} else {
    <<< "fail!" >>>;
}

while( true )
{
    0 => b.pos;
    400::ms => now;
}
