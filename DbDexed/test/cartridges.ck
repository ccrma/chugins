DbDexed inst => dac;

[60, 62, 64, 65, 67, 69, 71] @=> int notes[];
["../presets/SynprezFM_01.syx",
 "../presets/SynprezFM_02.syx",
 "../presets/SynprezFM_03.syx",
 "../presets/SynprezFM_04.syx",
 "../presets/SynprezFM_05.syx",
 "../presets/SynprezFM_06.syx",
 "../presets/SynprezFM_07.syx",
 "../presets/SynprezFM_08.syx",
 "../presets/SynprezFM_09.syx",
 "../presets/SynprezFM_10.syx",
 "../presets/SynprezFM_11.syx",
 "../presets/SynprezFM_12.syx",
 "../presets/SynprezFM_13.syx",
 "../presets/SynprezFM_14.syx",
 "../presets/SynprezFM_15.syx",
 "../presets/SynprezFM_16.syx",
 "../presets/SynprezFM_17.syx",
 "../presets/SynprezFM_18.syx",
 "../presets/SynprezFM_19.syx",
 "../presets/SynprezFM_20.syx",
 "../presets/SynprezFM_21.syx",
 "../presets/SynprezFM_22.syx",
 "../presets/SynprezFM_23.syx",
 "../presets/SynprezFM_24.syx",
 "../presets/SynprezFM_25.syx",
 "../presets/SynprezFM_26.syx",
 "../presets/SynprezFM_27.syx",
 "../presets/SynprezFM_28.syx",
 "../presets/SynprezFM_29.syx",
 "../presets/SynprezFM_30.syx",
 "../presets/SynprezFM_31.syx",
 "../presets/SynprezFM_32.syx"
 ] @=> string cartridges[];

.05::second => dur noteDur;
for(int c;c<cartridges.size();c++)
{
    cartridges[c] => string cnm;
    <<<"Cartridge", cnm, "---------------">>>;
    if(0 != inst.loadCartridge(cnm))
        break;

    for(int l;l<inst.getNumPresets();l++)
    {
        <<<inst.getPresetName(l), l>>>;
        inst.selectPreset(l);
        for(int i;i<notes.size();i++)
        {
            notes[i] => int n;
            inst.noteOn(n, 1);
            noteDur => now;
            inst.noteOff(n, 1);
            noteDur => now;
        }
    }
}

2::second => now; // ring


