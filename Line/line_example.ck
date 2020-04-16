Line line => blackhole;

0 => int c;
while(true)
{
    if(c > 20)
        0 => c;
    if(!c)
    {
        line.time(Math.random2f(0, 5));
        line.target(Math.random2f(-5, 5));
        <<< "TIME: " + line.time(), " || TARGET: " + line.target() >>>;
    }
        
    <<< line.last() >>>;
    c++;
    100::ms => now;
}