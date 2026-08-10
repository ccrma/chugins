// test_ir_correctness.ck
// Verify that ConvRev produces expected convolution output

ConvRev rev;
// Impulse response length
4 => int order;
rev.setOrder(order);

// Create a known IR: [0.5, 0.25, 0.1, 0.05]
// Note: ConvRev will normalize this so max peak is 0.5
// Max peak is 0.5, so scale factor is 0.5/0.5 = 1.0. 
// So effective IR should remain [0.5, 0.25, 0.1, 0.05]
rev.setCoeff(0, 0.5);
rev.setCoeff(1, 0.25);
rev.setCoeff(2, 0.1);
rev.setCoeff(3, 0.05);

// Initialize
rev.init();

// Audio graph
Impulse imp => rev => blackhole;

// 1. Trigger impulse
1.0 => imp.next;

// Advance time by 1 sample and check output
1::samp => now;
if(Math.fabs(rev.last() - 0.5) > 0.0001) {
    <<< "FAIL: Sample 0 expected 0.5, got", rev.last() >>>;
} else {
    <<< "PASS: Sample 0 matches." >>>;
}

1::samp => now;
if(Math.fabs(rev.last() - 0.25) > 0.0001) {
    <<< "FAIL: Sample 1 expected 0.25, got", rev.last() >>>;
} else {
    <<< "PASS: Sample 1 matches." >>>;
}

1::samp => now;
if(Math.fabs(rev.last() - 0.1) > 0.0001) {
    <<< "FAIL: Sample 2 expected 0.1, got", rev.last() >>>;
} else {
    <<< "PASS: Sample 2 matches." >>>;
}

1::samp => now;
if(Math.fabs(rev.last() - 0.05) > 0.0001) {
    <<< "FAIL: Sample 3 expected 0.05, got", rev.last() >>>;
} else {
    <<< "PASS: Sample 3 matches." >>>;
}

// 2. Test Normalization logic
// Set IR to [2.0, 1.0, 0.0, 0.0]
// Max is 2.0. Scale factor should be 0.5 / 2.0 = 0.25
// expected IR: [0.5, 0.25, 0.0, 0.0]

rev.setCoeff(0, 2.0);
rev.setCoeff(1, 1.0);
rev.setCoeff(2, 0.0);
rev.setCoeff(3, 0.0);
rev.init();

1.0 => imp.next;

1::samp => now;
if(Math.fabs(rev.last() - 0.5) > 0.0001) {
    <<< "FAIL: Norm Sample 0 expected 0.5, got", rev.last() >>>;
} else {
    <<< "PASS: Norm Sample 0 matches." >>>;
}

1::samp => now;
if(Math.fabs(rev.last() - 0.25) > 0.0001) {
    <<< "FAIL: Norm Sample 1 expected 0.25, got", rev.last() >>>;
} else {
    <<< "PASS: Norm Sample 1 matches." >>>;
}

<<< "Correctness test finished." >>>;
