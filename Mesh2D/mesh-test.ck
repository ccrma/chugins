Mesh2D mesh1 => dac.left;
Mesh2D mesh2 => dac.right;
0.5 => mesh1.gain => mesh2.gain;

while (true)
{
	Math.random2(2,12) => mesh1.x;
	Math.random2(2,12) => mesh1.y;
	Math.randomf() => mesh1.xpos;
	Math.randomf() => mesh1.ypos;
	1 => mesh1.noteOn;
	250::ms => now;
	Math.random2(2,12) => mesh2.x;
	Math.random2(2,12) => mesh2.y;
	Math.randomf() => mesh2.xpos;
	Math.randomf() => mesh2.ypos;
	1 => mesh2.noteOn;
	250::ms => now;
	
}