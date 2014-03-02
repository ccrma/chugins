/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OCOMB_H_
#define _OCOMB_H_ 1

// Non-interpolating comb filter class.  This produces the buzzy sound
// of the classic cmix comb filter.  Can be used with dynamically changing
// delay times, but with glitchy results -- sometimes desirable!  For
// smoother, duller results, use the Ozcomb object.  Ocomb is a translation
// of cmix combset/comb.  -JGG, 7/8/04

class Odelay;

class Ocomb
{
public:

	// Use this constructor when you don't intend to change the loopTime
	// dynamically.  <reverbTime> must be greater than zero.

	Ocomb(float SR, float loopTime, float reverbTime);

	// Use this constructor when you do intend to change the loopTime
	// dynamically.  Set <defaultLoopTime> to be 1.0/(lowest resonated 
	// frequency that you expect to use).  It must be greater
	// than or equal to loopTime.  <reverbTime> must be greater than zero.
	// 'del' is left NULL unless you want to create your own subclass of
	// Odelay and pass it in.  This ctor is used by subclasses as well.

	Ocomb(float SR, float loopTime, float defaultLoopTime, float reverbTime, Odelay *del=0);

	~Ocomb();

	void clear();

	// setReverbTime can be called repeatedly while running.
	// <reverbTime> must be greater than zero.

	void setReverbTime(float reverbTime);

	// There are two next() methods: use the first for non-changing delay
	// times, the second for changing delay times.  The second uses a non-
	// interpolating delay line, so there will be glitches when changing
	// pitch, esp. when glissing downward.

	float next(float input);

	// Note: loopTime is expressed here in terms of samples.

	float next(float input, float delaySamps);

	// Current frequency of comb
	
	float frequency() const;

protected:
	// This is called by derived classes which create their own delays
	Ocomb(Odelay *theDelay, float SR);

private:
	void init(float loopTime, float defaultLoopTime, float reverbTime, Odelay *del);

private:
	Odelay *_delay;
	float _sr;
	float _gain;
	float _lastout;
	float _delsamps;
};

#endif // _OCOMB_H_
