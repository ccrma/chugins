SinOsc sine => Clip clip => blackhole;
sine.freq(.5);

// set low and hi
clip.range(-0.5, 0.5);
<<< "---\n","LOW: " + clip.getLow(), " || HI: " + clip.getHi(), "\n---\n" >>>;

1.5::second => now;

while(true)
{
    <<< clip.last() >>>;
    50::ms => now;
}