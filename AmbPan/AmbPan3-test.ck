
CNoise noise => AmbPan3 pan => dac;

0.1 => noise.gain;
"pink" => noise.mode;

pi/2 => pan.elevation;

while(true)
{
    //pan.azimuth()+pi/1024 => pan.azimuth;
    //pan.elevation() + pi/512 => pan.elevation;
    5::ms => now;
}

