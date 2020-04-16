Phasor phasor => Relational R => blackhole;
phasor.freq(0.25);
R.operator(2);  // 0. ==, 1. !=, 2. >, 3. <, 4. >=, 5. <=
R.operand(0.5);

<<< "OPERATOR: " + R.operator(), "|| OPERAND: " + R.operand() >>>;

second => now;

while(true)
{
    <<< phasor.last(), R.last() >>>;
    100::ms => now;
}