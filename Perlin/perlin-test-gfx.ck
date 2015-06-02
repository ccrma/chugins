// note: requires chugl

Perlin perlin[2];

chugl gfx;
gfx.openWindow(512, 512);
gfx.width() => float WIDTH;
gfx.height() => float HEIGHT;

0 => float pos;
3 => float velocity;
0.01 => float scale;

gfx.hsv(Math.random2f(0,1), Math.random2f(0.4, 0.6), Math.random2f(0.7, 0.9));

while(true)
{
    for(0 => int i; i < WIDTH; i++)
    {
        perlin[0].noise1((pos+i)*scale) => float h;
        Std.scalef(h, -1, 1, 0, HEIGHT) => float y;
        gfx.line(i, 0, i, y);
    }
    
    velocity +=> pos;
    
    (1.0/60.0)::second => now;
}