#include "VAOnePoleFilter.h"
#include <math.h>

#define  pi 3.1415926535897932384626433832795

CVAOnePoleFilter::CVAOnePoleFilter(void)
{
	m_fAlpha = 1.0;
	m_fBeta = 1.0;

	m_uFilterType = LPF1;
	reset();
}

CVAOnePoleFilter::~CVAOnePoleFilter(void)
{
}

// recalc coeffs -- NOTE: not used for Korg35 Filter
void CVAOnePoleFilter::updateFilter()
{
	float wd = 2*pi*m_fFc;          
	float T  = 1/m_fSampleRate;             
	float wa = (2/T)*tan(wd*T/2); 
	float g  = wa*T/2;            

	m_fAlpha = g/(1.0 + g);
}

// do the filter
float CVAOnePoleFilter::doFilter(float xn)
{
	// calculate v(n)
	float vn = (xn - m_fZ1)*m_fAlpha;

	// form LP output
	float lpf = vn + m_fZ1;

	// update memory
	m_fZ1 = vn + lpf;

	// do the HPF
	float hpf = xn - lpf;

	if(m_uFilterType == LPF1)
		return lpf;
	else if(m_uFilterType == HPF1)
		return hpf;

	// default
	return lpf;

}


