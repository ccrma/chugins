/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "Ocomb.h"
#include "Odelay.h"

#include <math.h>
#include <assert.h>

Ocomb::Ocomb(float SR, float loopTime, float reverbTime)
	: _delay(0), _sr(SR), _lastout(0.0)
{
	init(loopTime,
		 loopTime,
		 reverbTime,
		 new Odelay(1 + (long) (loopTime * _sr + 0.5)));
}

Ocomb::Ocomb(float SR, float loopTime, float defaultLoopTime,
			 float reverbTime, Odelay *theDelay)
	: _delay(0), _sr(SR), _lastout(0.0)
{
	init(loopTime,
		 defaultLoopTime,
		 reverbTime, 
		 theDelay ? theDelay : new Odelay(1 + (long) (defaultLoopTime * _sr + 0.5)));
}

void Ocomb::init(float loopTime, float defaultLoopTime, float reverbTime, Odelay *delay)
{
	assert(defaultLoopTime > 0.0);
	assert(defaultLoopTime >= loopTime);
	_delay = delay;
	_delsamps = loopTime * _sr;
	_delay->setdelay(_delsamps);
	setReverbTime(reverbTime);
}

Ocomb::~Ocomb()
{
	delete _delay;
}

void Ocomb::clear()
{
	_delay->clear();
	_lastout = 0.0;
};

void Ocomb::setReverbTime(float reverbTime)
{
	assert(reverbTime > 0.0);
	_gain = powf(0.001f, (float)((_delsamps / _sr) / reverbTime));
}

float Ocomb::next(float input)
{
	float tmp = input + (_gain * _delay->last());
	_lastout = _delay->next(tmp);
	return _lastout;
}

float Ocomb::next(float input, float delaySamps)
{
	if (delaySamps != _delsamps) {
		_delsamps = delaySamps;
		_delay->setdelay(_delsamps);
	}
	float tmp = input + (_gain * _delay->last());
	_lastout = _delay->next(tmp);
	return _lastout;
}

float Ocomb::frequency() const
{
	float delay = _delay->delay();
	return (delay > 0.0f) ? _sr / delay : 0.0f;
}
