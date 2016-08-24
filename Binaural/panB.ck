SndBuf b => Binaural binaural => dac;
"special:dope" => b.read;

while( true )
{
    0 => b.pos;
    200::ms => now;
}
