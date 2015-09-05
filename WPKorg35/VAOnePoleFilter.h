#pragma once

typedef unsigned int        UINT;

class CVAOnePoleFilter
{
public:
	CVAOnePoleFilter(void);
	~CVAOnePoleFilter(void);
	
	// common variables
	float m_fSampleRate;	/* sample rate*/
	float m_fFc;			/* cutoff frequency */
	
	UINT m_uFilterType;		/* filter selection */
	enum{LPF1,HPF1}; /* one short string for each */

	// Trapezoidal Integrator Components
	float m_fAlpha;			// Feed Forward coeff
	float m_fBeta;			// Feed Back coeff

	// provide access to our feedback output
	float getFeedbackOutput(){return m_fZ1*m_fBeta;}

	// -- CFilter Overrides --
	virtual void reset(){m_fZ1 = 0;}

	// recalc the coeff -- NOTE: not used for Korg35 Filter
	virtual void updateFilter();
	
	// do the filter
	virtual float doFilter(float xn);

protected:
	float m_fZ1;		// our z-1 storage location
};

