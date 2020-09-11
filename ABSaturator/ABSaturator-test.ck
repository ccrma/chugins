
adc => Delay d => ABSaturator sat => dac;

20 => sat.driveDb; // in dB
4 => sat.dcOffset;

while(true)
{
    1::second => now;
}
