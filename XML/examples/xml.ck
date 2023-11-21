// this code takes in the "values.xml" file, reads it, and plays the content.

XML xml; // The xml object

me.dir() + "values.xml" => xml.open; // the path to the xml file

xml.pushTag("chuck",0); // push the first tag on the file

SinOsc s => dac;

for(int i; i < xml.getNumTags("a"); i++){ // iterating through the tags in xml file

    if(xml.tagExists("a",i)){ // check if the tag exists
        xml.pushTag("a",i); // push the tag

        xml.getIntValue("freq",0,0) => int note; // get the frequency value
        xml.getFloatValue("gain",0,0) => float gain; // get the gain value
        xml.getIntValue("dur",0,0) => int d; // get the duration value
        
        // play the sound
        Std.mtof(note) => s.freq;
        gain => s.gain;
        d::ms => now;

        <<< note, gain, d >>>;

        xml.popTag(); // pop the tag and move to the next
    }
}

