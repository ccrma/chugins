
#pragma once

// base class
#include "VAOnePoleFilterEx.h"

// abstract base class for RackAFX filters
class CVADiodeLadderFilter
{
public:
	// RackAFX Plug-In API Member Methods:
	// The followung 5 methods must be impelemented for a meaningful Plug-In
	//
	// 1. One Time Initialization
	CVADiodeLadderFilter();

	// 2. One Time Destruction
	virtual ~CVADiodeLadderFilter(void);

	// 3. The Prepare For Play Function is called just before audio streams
	bool prepareForPlay(float fs);

	// Add your code here: ----------------------------------------------------------- //
	CVAOnePoleFilterEx m_LPF1;
	CVAOnePoleFilterEx m_LPF2;
	CVAOnePoleFilterEx m_LPF3;
	CVAOnePoleFilterEx m_LPF4;

	double m_dGAMMA; // Gamma see App Note

	// our feedback S values (global)
	double m_dSG1; 
	double m_dSG2; 
	double m_dSG3; 
	double m_dSG4; 

	void reset()
	{
		m_LPF1.reset(); m_LPF2.reset();
		m_LPF3.reset(); m_LPF4.reset();

		m_LPF1.setFeedback(0.0); m_LPF2.setFeedback(0.0); 
		m_LPF3.setFeedback(0.0); m_LPF4.setFeedback(0.0); 
	}

	// recalc the coeffs
	void updateFilter();
	
	// do the filter
	double doFilter(double xn);

    float m_nSampleRate;
	double m_dFc;
	double m_dK;
	double m_dSaturation;
	UINT m_NonLinearProcessing;
	enum{OFF,ON};
	UINT m_uNLPType;
	enum{NORM,REG};

};














