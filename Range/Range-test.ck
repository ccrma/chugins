SinOsc sin => Range scale => blackhole;
sin => Gain g => blackhole;

(-1, 1, 0, 2) => scale.range;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

(0, 1, 1, 1 ) => scale.radius;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

(-1, 1) => scale.inRange;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

(0, 2) => scale.outRange;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

(0, 1) => scale.inRadius;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

(1, 1) => scale.outRadius;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

// test exceeding clip bounds
2 => sin.gain;
validate(1.0);
validateRange(-1, 1, 0, 2);
validateRadius(0, 1, 1, 1);

// test with hard clipping
1 => scale.clip;
validateClip(1.0, 0, 2);

fun void validate(float offset) {
	now + 1::second => time later;
	while(now < later) {
1::samp => now;
		(g.last()) + offset => float want;
		if (!within(scale.last(), want, 0.001)) {
			<<< "FAILURE, in = ", g.last(), "got = ", scale.last(), "want = ", want >>>;
		}
	}
}

fun void validateClip(float offset, float min, float max) {
	now + 1::second => time later;
	while(now < later) {
		1::samp => now;
		(g.last()) + offset => float want;
		Math.max(want, min) => want;
		Math.min(want, max) => want;
		if (!within(scale.last(), want, 0.001)) {
			<<< "FAILURE, in = ", g.last(), "got = ", scale.last(), "want = ", want >>>;
		}
	}
}


// check that two floats are within err of each other
fun int within(float a, float b, float err) {
	Math.fabs(a - b) => float result;

	if (result < err) {
		return true;
	}
	return false;
}

fun void validateRange(float inMin, float inMax, float outMin, float outMax) {
	if (!within(scale.inMin(), inMin, 0.001)) <<< "inMin FAILURE", scale.inMin(), inMin >>>;
	if (!within(scale.inMax(), inMax, 0.001)) <<< "inMax FAILURE", scale.inMax(), inMax >>>;
	if (!within(scale.outMin(), outMin, 0.001)) <<< "outMin FAILURE", scale.outMin(), outMin >>>;
	if (!within(scale.outMax(), outMax, 0.001)) <<< "outMax FAILURE", scale.outMax(), outMax >>>;
}

fun void validateRadius(float inCenter, float inRadius, float outCenter, float outRadius) {
	if (!within(scale.inCenter(), inCenter, 0.001)) <<< "inCenter FAILURE", scale.inCenter(), inCenter >>>;
	if (!within(scale.inRad(), inRadius, 0.001)) <<< "inRadius FAILURE", scale.inRad(), inRadius >>>;
	if (!within(scale.outCenter(), outCenter, 0.001)) <<< "outCenter FAILURE", scale.outCenter(), outCenter >>>;
	if (!within(scale.outRad(), outRadius, 0.001)) <<< "outRadius FAILURE", scale.outRad(), outRadius >>>;
}
