//***********************************************************************************************
// Relativistic Oscillator kernel
//
// Based on the BogAudio FM-OP oscillator by Matt Demanett
//  99.9% of the code here is by Matt Demanett
// See ./LICENSE.txt for all licenses
//
//***********************************************************************************************

#pragma once

// #include "Geodesics.hpp"
#include <cassert>
#include <cmath>

//-----------------------------------------------------------------------------
// SlewLimiter
//-----------------------------------------------------------------------------

// Modified by Marc Boulé to allow asymetrical slewing

struct SlewLimiter
{
	float _deltaUp;
	float _deltaDown;
	float _last = 0.0f;

	SlewLimiter(float sampleRate = 1000.0f, float milliseconds = 1.0f, float range = 10.0f)
	{
		setParams(sampleRate, milliseconds, range);
	}

	void setParams(float sampleRate = 1000.0f, float milliseconds = 1.0f, float range = 10.0f);
	void setParams2(float sampleRate = 1000.0f, float millisecondsUp = 1.0f, float millisecondsDown = 1.0f, float range = 10.0f);
	inline float next(float sample)
	{
		return _last = next(sample, _last);
	}
	float next(float sample, float last);
};

//-----------------------------------------------------------------------------
// StaticSineTable
//-----------------------------------------------------------------------------

class Table
{
protected:
	int _length = 0;
	float *_table = NULL;

public:
	Table(int n = 10)
	{
		assert(n > 0);
		assert(n <= 16);
		_length = 1 << n;
	}
	virtual ~Table()
	{
		if (_table)
		{
			delete[] _table;
		}
	}

	inline int length() const { return _length; }

	inline float value(int i) const
	{
		assert(i >= 0 && i < _length);
		assert(_table);
		return _table[i];
	}

	void generate();

protected:
	virtual void _generate() = 0;
};

template <class T, int N>
class StaticTable
{
private:
	Table *_table = NULL;

	StaticTable()
	{
	}
	~StaticTable()
	{
		if (_table)
		{
			delete _table;
		}
	}

public:
	StaticTable(const StaticTable &) = delete;
	void operator=(const StaticTable &) = delete;

	static const Table &table()
	{
		static StaticTable<T, N> instance;
		if (!instance._table)
		{
			instance._table = new T(N);
			instance._table->generate();
		}
		return *instance._table;
	}
};

struct SineTable : Table
{
	SineTable(int n = 10) : Table(n) {}
	void _generate() override;
};
struct StaticSineTable : StaticTable<SineTable, 12>
{
};

//-----------------------------------------------------------------------------
// Decimator
//-----------------------------------------------------------------------------

struct Decimator
{
	Decimator() {}
	virtual ~Decimator() {}

	virtual void setParams(float sampleRate, int factor) = 0;
	virtual float next(const float *buf) = 0;
};

struct CICDecimator : Decimator
{
	typedef int64_t T;
	static constexpr T scale = ((T)1) << 32;
	int _stages;
	T *_integrators = nullptr;
	T *_combs = nullptr;
	int _factor = 0;
	float _gainCorrection;

	CICDecimator(int stages = 4, int factor = 8);
	virtual ~CICDecimator();
	CICDecimator(const CICDecimator &srcc);
	void copyFrom(const CICDecimator &srcc);
	CICDecimator &operator=(const CICDecimator &srcc);
	void setParams(float sampleRate, int factor) override final;
	float next(const float *buf) override;
};

//-----------------------------------------------------------------------------
// SineTableOscillator
//-----------------------------------------------------------------------------

struct Generator
{
	float _current = 0.0;

	Generator() {}
	virtual ~Generator() {}

	float current()
	{
		return _current;
	}

	float next()
	{
		return _current = _next();
	}

	virtual float _next() = 0;
};

struct Oscillator
{
	float _sampleRate;
	float _frequency;

	Oscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f)
		: _sampleRate(sampleRate > 1.0 ? sampleRate : 1.0), _frequency(frequency)
	{
	}
	virtual ~Oscillator() {}

	void setSampleRate(float sampleRate)
	{
		if (_sampleRate != sampleRate && sampleRate >= 1.0)
		{
			_sampleRate = sampleRate;
			_sampleRateChanged();
		}
	}

	virtual void _sampleRateChanged() {}

	void setFrequency(float frequency)
	{
		if (_frequency != frequency)
		{
			_frequency = frequency;
			_frequencyChanged();
		}
	}

	virtual void _frequencyChanged() {}
};

struct OscillatorGenerator : Oscillator, Generator
{
	OscillatorGenerator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f)
		: Oscillator(sampleRate, frequency)
	{
	}
};

struct Phasor : OscillatorGenerator
{
	typedef uint32_t phase_t;
	typedef int64_t phase_delta_t;
	static constexpr phase_t maxPhase = UINT32_MAX;
	static constexpr float twoPI = 2.0f * float(M_PI);
	static constexpr float maxSampleWidth = 0.25f;

	phase_delta_t _delta;
	phase_t _phase = 0;
	float _sampleWidth = 0.0f;
	phase_t _samplePhase = 0;

	Phasor(
		float sampleRate = 1000.0f,
		float frequency = 100.0f,
		float initialPhase = 0.0f)
		: OscillatorGenerator(sampleRate, frequency)
	{
		setPhase(initialPhase);
		_update();
	}

	void _sampleRateChanged() override
	{
		_update();
	}

	void _frequencyChanged() override
	{
		_update();
	}

	phase_t getPhase() { return _phase; }
	void setSampleWidth(float sw);
	void resetPhase();
	void setPhase(float radians);
	void setPhase(phase_t givenPhase) { _phase = givenPhase; }
	float nextFromPhasor(const Phasor &phasor, phase_delta_t offset = 0);
	inline float nextForPhase(phase_t phase) { return _nextForPhase(phase); }
	void _update();
	inline void advancePhase() { _phase += _delta; }
	inline void advancePhase(int n)
	{
		assert(n > 0);
		_phase += n * _delta;
	}
	float _next() override final;
	virtual float _nextForPhase(phase_t phase);

	inline static phase_delta_t radiansToPhase(float radians) { return (radians / twoPI) * (float)maxPhase; }
	inline static float phaseToRadians(phase_t phase) { return (phase / (float)maxPhase) * twoPI; }
};

struct TablePhasor : Phasor
{
	const Table &_table;
	int _tableLength;

	TablePhasor(
		const Table &table,
		double sampleRate = 1000.0f,
		double frequency = 100.0f)
		: Phasor(sampleRate, frequency), _table(table), _tableLength(table.length())
	{
	}

	float _nextForPhase(phase_t phase) override;
};

struct SineTableOscillator : TablePhasor
{
	SineTableOscillator(
		float sampleRate = 1000.0f,
		float frequency = 100.0f)
		: TablePhasor(StaticSineTable::table(), sampleRate, frequency)
	{
	}
};

//-----------------------------------------------------------------------------
// FMOp
//-----------------------------------------------------------------------------

struct FMOp
{
	const float amplitude = 5.0f;
	const int modulationSteps = 100;
	static constexpr int oversample = 8;
	const float oversampleMixIncrement = 0.01f;
	int _steps = 0;
	float _feedbackDelayedSample = 0.0f;
	float _maxFrequency = 0.0f;
	float _buffer[oversample] = {};
	float _oversampleMix = 0.0f;
	Phasor _phasor;
	SineTableOscillator _sineTable;
	CICDecimator _decimator;
	SlewLimiter _feedbackSL;

	FMOp(float _sampleRate)
	{
		onSampleRateChange(_sampleRate);
		onReset();
	}

	void onReset();
	void onSampleRateChange(float newSampleRate);
	float step(float voct, float momentum, float fmDepth = 0.0f, float fmInput = 0.0f);
};
