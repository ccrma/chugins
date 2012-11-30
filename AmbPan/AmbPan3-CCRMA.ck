
CNoise noise => AmbPan3 pan => dac;

0.1 => noise.gain;
"pink" => noise.mode;

// W  X  Y  U  V   P   Q  Z  R  S  T  K   L   M   N   O
  [0, 1, 2, 7, 8, 14, 15, 3, 4, 5, 6, 9, 10, 11, 12, 13] => pan.channelMap;

//pi/2 => pan.elevation;

while(true)
{
    pan.azimuth()+pi/1024 => pan.azimuth;
    //pan.elevation() + pi/512 => pan.elevation;
    5::ms => now;
}

