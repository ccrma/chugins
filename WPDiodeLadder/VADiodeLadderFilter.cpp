/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "VADiodeLadderFilter.h"
#include <math.h>
#define  pi 3.1415926535897932384626433832795

CVADiodeLadderFilter::CVADiodeLadderFilter()
{

	// Finish initializations here
	m_dGAMMA = 0.0;
	m_dK = 0.0;

	// our feedback S values (global)
	m_dSG1 = 0.0; 
	m_dSG2 = 0.0;
	m_dSG3 = 0.0; 
	m_dSG4 = 0.0; 

	// Filter coeffs that are constant
	// set a0s
	m_LPF1.m_da0 = 1.0;
	m_LPF2.m_da0 = 0.5;
	m_LPF3.m_da0 = 0.5;
	m_LPF4.m_da0 = 0.5;

	// last LPF has no feedback path
	m_LPF4.m_dGamma = 1.0;
	m_LPF4.m_dDelta = 0.0;
	m_LPF4.m_dEpsilon = 0.0;
	m_LPF4.setFeedback(0.0);

}

CVADiodeLadderFilter::~CVADiodeLadderFilter(void)
{

}

bool CVADiodeLadderFilter::prepareForPlay(float fs)
{

	// this flushes all storage registers in filters including feedback register
	reset();

    m_nSampleRate = fs;
    m_LPF1.m_dSampleRate = m_nSampleRate;
    m_LPF2.m_dSampleRate = m_nSampleRate;
    m_LPF3.m_dSampleRate = m_nSampleRate;
    m_LPF4.m_dSampleRate = m_nSampleRate;

	// set the initial coeffs
	updateFilter();

	return true;
}

void CVADiodeLadderFilter::updateFilter()
{
	// calculate alphas
	double wd = 2*pi*m_dFc;          
	double T  = 1/(float)m_nSampleRate;             
	double wa = (2/T)*tan(wd*T/2); 
	double g = wa*T/2;  

	// Big G's
	double G1, G2, G3, G4;

	G4 = 0.5*g/(1.0 + g);
	G3 = 0.5*g/(1.0 + g - 0.5*g*G4);
	G2 = 0.5*g/(1.0 + g - 0.5*g*G3);
	G1 = g/(1.0 + g - g*G2);
	
	// our big G value GAMMA
	m_dGAMMA = G4*G3*G2*G1;
	
	m_dSG1 =  G4*G3*G2; 
	m_dSG2 =  G4*G3; 
	m_dSG3 =  G4; 
	m_dSG4 =  1.0; 

	// set alphas
	m_LPF1.m_dAlpha = g/(1.0 + g);
	m_LPF2.m_dAlpha = g/(1.0 + g);
	m_LPF3.m_dAlpha = g/(1.0 + g);
	m_LPF4.m_dAlpha = g/(1.0 + g);

	// set betas
	m_LPF1.m_dBeta = 1.0/(1.0 + g - g*G2);
	m_LPF2.m_dBeta = 1.0/(1.0 + g - 0.5*g*G3);
	m_LPF3.m_dBeta = 1.0/(1.0 + g - 0.5*g*G4);
	m_LPF4.m_dBeta = 1.0/(1.0 + g);
	
	// set gammas
	m_LPF1.m_dGamma = 1.0 + G1*G2;
	m_LPF2.m_dGamma = 1.0 + G2*G3;
	m_LPF3.m_dGamma = 1.0 + G3*G4;
	// m_LPF4.m_dGamma = 1.0; // constant - done in constructor
	
	// set deltas
	m_LPF1.m_dDelta = g;
	m_LPF2.m_dDelta = 0.5*g;
	m_LPF3.m_dDelta = 0.5*g;
	// m_LPF4.m_dDelta = 0.0; // constant - done in constructor

	// set epsilons
	m_LPF1.m_dEpsilon = G2;
	m_LPF2.m_dEpsilon = G3;
	m_LPF3.m_dEpsilon = G4;
	// m_LPF4.m_dEpsilon = 0.0; // constant - done in constructor
}

// do the filter
double CVADiodeLadderFilter::doFilter(double xn)
{
//	m_LPF4.setFeedback(0.0); // constant - done in constructor
	m_LPF3.setFeedback(m_LPF4.getFeedbackOutput());
	m_LPF2.setFeedback(m_LPF3.getFeedbackOutput());
	m_LPF1.setFeedback(m_LPF2.getFeedbackOutput());

	// form input
	double SIGMA = m_dSG1*m_LPF1.getFeedbackOutput() + 
				   m_dSG2*m_LPF2.getFeedbackOutput() +
				   m_dSG3*m_LPF3.getFeedbackOutput() +
				   m_dSG4*m_LPF4.getFeedbackOutput();

	// "cheap" nonlinear model; just process input
	if(m_NonLinearProcessing == ON)
	{
		// the 1/tanh(sat) is to normalize the function so x = [-1..+1] --> y = [-1..+1]
		// Normalized Version
		if(m_uNLPType == NORM)
			xn = (1.0/tanh(m_dSaturation))*tanh(m_dSaturation*xn);
		else
			xn = tanh(m_dSaturation*xn);
	}

	// form the input to the loop
	double un = (xn - m_dK*SIGMA)/(1 + m_dK*m_dGAMMA);

	// cascade of series filters
	return m_LPF4.doFilter(m_LPF3.doFilter(m_LPF2.doFilter(m_LPF1.doFilter(un))));
}
