Step step => Scaler s => blackhole;


// set the range 0-1 to 1-2
(0, 1, 1, 2) => s.range;


if (s.inMin() != 0 || s.inMax() != 1 || s.outMin() != 1 || s.outMax() != 2) {
    <<< "failed" >>>;
}


0 => step.next;

1::samp => now;
if (s.last() != 1.0) {
    <<< "failed", s.last() >>>;
}

// scales linearly with no clipping
2 => step.next;
1::samp => now;
if (s.last() != 3.0) {
    <<< "failed", s.last() >>>;
}



