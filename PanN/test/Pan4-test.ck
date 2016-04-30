Step step => Pan4 pan => blackhole;
1 => step.next;

// Channels are presumed to be arranged in space like so:
// 0   1
//   ^  
// 2   3
// ^ represents listener facing forward

// therefore a panning of 0 will produce
// 1   0
//   ^
// 0   0
// in the channels as labeled above
// and the following gains for output channels 0-3:
// [1, 0, 0, 0]

// a panning of 0.5 will produce for X = sqrt(2)
// X   X
//   ^
// 0   0
// and the following gains for output channels 0-3:
// [X, X, 0, 0]

// a panning of 1.5 will produce for X = sqrt(2)
// 0   X
//   ^
// 0   X
// and the following gains for output channels 0-3:
// [0, X, 0, X]

0 => pan.pan;
1::samp => now;
if(pan.chan(0).last() != 1 && pan.chan(1).last() != 0)
{
    <<< "failure 0" >>>;
    me.exit();
}

1 => pan.pan;
1::samp => now;
if(pan.chan(1).last() != 1 && pan.chan(2).last() != 0)
{
    <<< "failure 1" >>>;
    me.exit();
}

2 => pan.pan;
1::samp => now;
if(pan.chan(3).last() != 1 && pan.chan(3).last() != 0)
{
    <<< "failure 2" >>>;
    me.exit();
}

3 => pan.pan;
1::samp => now;
if(pan.chan(2).last() != 1 && pan.chan(0).last() != 0)
{
    <<< "failure 3" >>>;
    me.exit();
}

<<< "success" >>>;
