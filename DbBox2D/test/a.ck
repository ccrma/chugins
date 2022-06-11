DbBox2D b;
100::ms => now;

b.worldBegin(#(0, -10));
b.newCircle(#(px, py), radius);
b.newRect( #(px, py), #(xlen, ylen), angle);
b.worldEnd();

for(int i;i<100;i++)
{
    b => now;
}
b.done();
