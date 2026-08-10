// test_convrev_repro.ck
// Minimal reproduction of ConvRev usage to test stability and correctness

ConvRev rev;
// Impulse response length
1024 => int order;
rev.setOrder(order);

// Create a simple impulse
0 => int i;
rev.setCoeff(0, 1.0); // Dirac delta at 0
// Clear rest
for(1 => i; i < order; i++) {
    rev.setCoeff(i, 0.0);
}

// Initialize
rev.init();

// Audio graph
Impulse imp => rev => dac;

// Trigger impulse
1.0 => imp.next;

// Let it run for a bit
100::ms => now;

<<< "ConvRev test finished without crash." >>>;
