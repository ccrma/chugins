Xml xml;

me.dir() + "values.xml" => xml.open;

xml.pushTag("chuck",0);

SinOsc s => dac;

for(int i; i < xml.getNumTags("a"); i++){

    if(xml.tagExists("a",i)){
        xml.pushTag("a",i);

        xml.getIntValue("freq",0,0) => int note;
        xml.getFloatValue("gain",0,0) => float gain;
        xml.getIntValue("dur",0,0) => int d;

        Std.mtof(note) => s.freq;
        gain => s.gain;
        d::ms => now;

        <<< note, gain, d >>>;

        xml.popTag();
    }
}

