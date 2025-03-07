//***********************************************************************************************
// Relativistic Oscillator kernel
//
// Based on the BogAudio FM-OP oscillator by Matt Demanett
//  99.9% of the code here is by Matt Demanett
// See ./LICENSE.txt for all licenses
//
//***********************************************************************************************

#include "EnergyOsc.hpp"

//-----------------------------------------------------------------------------
// SlewLimiter
//-----------------------------------------------------------------------------

void SlewLimiter::setParams(float sampleRate, float milliseconds, float range)
{
	assert(sampleRate > 0.0f);
	assert(milliseconds >= 0.0f);
	assert(range > 0.0f);
	_deltaUp = range / ((milliseconds / 1000.0f) * sampleRate);
	_deltaDown = _deltaUp;
}

void SlewLimiter::setParams2(float sampleRate, float millisecondsUp, float millisecondsDown, float range)
{
	assert(sampleRate > 0.0f);
	assert(millisecondsUp >= 0.0f);
	assert(millisecondsDown >= 0.0f);
	assert(range > 0.0f);
	_deltaUp = range / ((millisecondsUp / 1000.0f) * sampleRate);
	_deltaDown = range / ((millisecondsDown / 1000.0f) * sampleRate);
}

float SlewLimiter::next(float sample, float last)
{
	if (sample > last)
	{
		return fmin(last + _deltaUp, sample);
	}
	return fmax(last - _deltaDown, sample);
}

//-----------------------------------------------------------------------------
// Amplifier
//-----------------------------------------------------------------------------

void Table::generate()
{
	if (!_table)
	{
		_table = new float[_length]{};
		_generate();
	}
}

void SineTable::_generate()
{
	const float twoPI = 2.0f * float(M_PI);
	for (int i = 0, j = _length / 4; i <= j; ++i)
	{
		_table[i] = std::sin(twoPI * (i / (float)_length));
	}
	for (int i = 1, j = _length / 4; i < j; ++i)
	{
		_table[i + j] = _table[j - i];
	}
	for (int i = 0, j = _length / 2; i < j; ++i)
	{
		_table[i + j] = -_table[i];
	}
}

//-----------------------------------------------------------------------------
// Decimator
//-----------------------------------------------------------------------------

CICDecimator::CICDecimator(int stages, int factor)
{
	assert(stages > 0);
	_stages = stages;
	_integrators = new T[_stages + 1]{};
	_combs = new T[_stages]{};
	setParams(0.0f, factor);
}

CICDecimator::~CICDecimator()
{
	delete[] _integrators;
	delete[] _combs;
}

CICDecimator::CICDecimator(const CICDecimator &srcc)
{
	copyFrom(srcc);
}
void CICDecimator::copyFrom(const CICDecimator &srcc)
{
	// just dummy for now, finish implementing if needed
	_stages = srcc._stages;
	if (_integrators != nullptr)
	{
		delete[] _integrators;
	}
	_integrators = new T[_stages + 1]{}; // should copy
	if (_combs != nullptr)
	{
		delete[] _combs;
	}
	_combs = new T[_stages]{}; // should copy
	setParams(0.0f, srcc._factor);
}

CICDecimator &CICDecimator::operator=(const CICDecimator &srcc)
{
	copyFrom(srcc);
	return *this;
}

void CICDecimator::setParams(float _sampleRate, int factor)
{
	assert(factor > 0);
	if (_factor != factor)
	{
		_factor = factor;
		_gainCorrection = 1.0f / (float)(pow(_factor, _stages));
	}
}

float CICDecimator::next(const float *buf)
{
	for (int i = 0; i < _factor; ++i)
	{
		_integrators[0] = buf[i] * scale;
		for (int j = 1; j <= _stages; ++j)
		{
			_integrators[j] += _integrators[j - 1];
		}
	}
	T s = _integrators[_stages];
	for (int i = 0; i < _stages; ++i)
	{
		T t = s;
		s -= _combs[i];
		_combs[i] = t;
	}
	return _gainCorrection * (s / (float)scale);
}

//-----------------------------------------------------------------------------
// SineTableOscillator
//-----------------------------------------------------------------------------

void Phasor::setSampleWidth(float sw)
{
	if (sw < 0.0f)
	{
		sw = 0.0f;
	}
	else if (sw > maxSampleWidth)
	{
		sw = maxSampleWidth;
	}
	if (_sampleWidth != sw)
	{
		_sampleWidth = sw;
		if (_sampleWidth > 0.001f)
		{
			_samplePhase = _sampleWidth * (float)maxPhase;
		}
		else
		{
			_samplePhase = 0;
		}
	}
}

void Phasor::resetPhase()
{
	_phase = 0;
}

void Phasor::setPhase(float radians)
{
	_phase = radiansToPhase(radians);
}

float Phasor::nextFromPhasor(const Phasor &phasor, phase_delta_t offset)
{
	offset += phasor._phase;
	if (_samplePhase > 0)
	{
		offset -= offset % _samplePhase;
	}
	return _nextForPhase(offset);
}

void Phasor::_update()
{
	_delta = ((phase_delta_t)((_frequency / _sampleRate) * (float)maxPhase)) % maxPhase;
}

float Phasor::_next()
{
	advancePhase();
	if (_samplePhase > 0)
	{
		return _nextForPhase(_phase - (_phase % _samplePhase));
	}
	return _nextForPhase(_phase);
}

float Phasor::_nextForPhase(phase_t phase)
{
	return phase;
}

float TablePhasor::_nextForPhase(phase_t phase)
{
	if (_tableLength >= 1024)
	{
		int i = (((((uint64_t)phase) << 16) / maxPhase) * _tableLength) >> 16;
		if (i >= _tableLength)
		{
			i %= _tableLength;
		}
		return _table.value(i);
	}

	float fi = (phase / (float)maxPhase) * _tableLength;
	int i = (int)fi;
	if (i >= _tableLength)
	{
		i %= _tableLength;
	}
	float v1 = _table.value(i);
	float v2 = _table.value(i + 1 == _tableLength ? 0 : i + 1);
	return v1 + (fi - i) * (v2 - v1);
}

//-----------------------------------------------------------------------------
// FMOp
//-----------------------------------------------------------------------------

const float referenceFrequency = 261.626; // C4; frequency at which Rack 1v/octave CVs are zero.

inline float cvToFrequency(float cv)
{
	return std::pow(2.0f, cv) * referenceFrequency;
}

void FMOp::onReset()
{
	_steps = modulationSteps;
	_phasor.resetPhase();
}

void FMOp::onSampleRateChange(const float newSampleRate)
{
	_steps = modulationSteps;
	_phasor.setSampleRate(newSampleRate);
	_decimator.setParams(newSampleRate, oversample);
	_maxFrequency = 0.475f * newSampleRate;
	_feedbackSL.setParams(newSampleRate, 5.0f, 1.0f);
}

#define clamp(x, a, b) fmin(fmax(x, a), b)

float FMOp::step(float voct, float momentum, float fmDepth, float fmInput)
{
	++_steps;
	if (_steps >= modulationSteps)
	{
		_steps = 0;
		float frequency = voct;
		// frequency += params[FINE_PARAM].value / 12.0f;
		frequency = cvToFrequency(frequency);
		// frequency *= ratio;
		frequency = clamp(frequency, -_maxFrequency, _maxFrequency);
		_phasor.setFrequency(frequency / (float)oversample);
	}

	float feedback = _feedbackSL.next(momentum);
	bool feedbackOn = feedback > 0.001f;

	float offset = 0.0f;
	if (feedbackOn)
	{
		offset = feedback * _feedbackDelayedSample;
	}

	bool depthOn = false;
	if (fmDepth != 0.0f)
	{
		offset += fmInput * fmDepth * 2.0f;
		depthOn = fmDepth > 0.001f;
	}

	float sample = 0.0f;

	Phasor::phase_delta_t o = Phasor::radiansToPhase(offset);
	if (feedbackOn || depthOn)
	{
		if (_oversampleMix < 1.0f)
		{
			_oversampleMix += oversampleMixIncrement;
		}
	}
	else if (_oversampleMix > 0.0f)
	{
		_oversampleMix -= oversampleMixIncrement;
	}

	if (_oversampleMix > 0.0f)
	{
		for (int i = 0; i < oversample; ++i)
		{
			_phasor.advancePhase();
			_buffer[i] = _sineTable.nextFromPhasor(_phasor, o);
		}
		sample = _oversampleMix * _decimator.next(_buffer);
	}
	else
	{
		_phasor.advancePhase(oversample);
	}
	if (_oversampleMix < 1.0f)
	{
		sample += (1.0f - _oversampleMix) * _sineTable.nextFromPhasor(_phasor, o);
	}

	return _feedbackDelayedSample = amplitude * sample;
}

#undef clamp
