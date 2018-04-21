// name: phaser.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust phaser => dac;

// evaluate Faust code
phaser.eval(`
  process = dm.phaser2_demo ;
`);

// print the parameters of the Faust object
phaser.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
