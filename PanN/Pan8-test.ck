
CNoise noise => Pan8 pan;
for(int i; i < 8; i++) pan.chan(i) => dac.chan(i);

"pink" => noise.mode;
0.1 => noise.gain;

0 => pan.pan;

while(true)
{
    pan.pan() + 0.01 => pan.pan;
    10::ms => now;
}
