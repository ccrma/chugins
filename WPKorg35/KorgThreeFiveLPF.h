/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Definition
	Copyright(c) Tritone Systems Inc. 2006-2012

	Your plug-in must implement the constructor,
	destructor and virtual Plug-In API Functions below.
*/

#pragma once

// base class
#include "VAOnePoleFilter.h"

class CKorgThreeFiveLPF
{
public:
	// RackAFX Plug-In API Member Methods:
	// The followung 5 methods must be impelemented for a meaningful Plug-In
	//
	// 1. One Time Initialization
	CKorgThreeFiveLPF();

	// 2. One Time Destruction
	virtual ~CKorgThreeFiveLPF(void);

	// 3. The Prepare For Play Function is called just before audio streams
	bool prepareForPlay(float fs);

	// Add your code here: ----------------------------------------------------------- //
	CVAOnePoleFilter m_LPF1;
	CVAOnePoleFilter m_LPF2;
	CVAOnePoleFilter m_HPF1;

	// fn to update when UI changes
	void updateFilters();

	// main do function
	double doFilter(double xn);

	// variables
	double m_dAlpha0;   // our u scalar value

	// enum needed for child members 
	enum{LPF1,HPF1}; /* one short string for each */
    float m_nSampleRate;
	double m_dFc;
	double m_dK;
	double m_dSaturation;
	UINT m_uNonLinearProcessing;
	enum{OFF,ON};

};









