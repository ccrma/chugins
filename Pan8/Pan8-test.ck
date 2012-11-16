
BlitSquare source => Pan8 pan => dac;

220 => source.freq;

while(true)
{
    pan.pan() + 0.01 => pan.pan;
    20::ms => now;
}
