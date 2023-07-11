/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "KorgThreeFiveLPF.h"
#include <math.h>
#define  pi 3.1415926535897932384626433832795

CKorgThreeFiveLPF::CKorgThreeFiveLPF()
{

	// Finish initializations here
	m_dAlpha0 = 0.0;

}

CKorgThreeFiveLPF::~CKorgThreeFiveLPF(void)
{

}

bool CKorgThreeFiveLPF::prepareForPlay(float fs)
{
	// set types
	m_LPF1.m_uFilterType = LPF1;
	m_LPF2.m_uFilterType = LPF1;
	m_HPF1.m_uFilterType = HPF1;
	
	// flush everything
	m_LPF1.reset();
	m_LPF2.reset();
	m_HPF1.reset();

	m_LPF1.m_fSampleRate = fs;
	m_LPF2.m_fSampleRate = fs;
	m_HPF1.m_fSampleRate = fs;

    m_nSampleRate = fs;

	// set initial coeff states
	updateFilters();
	return true;
}

void CKorgThreeFiveLPF::updateFilters()
{
	// use this is f you want to let filters update themselves;
	// since we calc everything here, it would be redundant

	// prewarp for BZT
	double wd = 2*pi*m_dFc;          
	double T  = 1/(double)m_nSampleRate;             
	double wa = (2/T)*tan(wd*T/2); 
	double g  = wa*T/2;    

	// G - the feedforward coeff in the VA One Pole
	float G = (float)(g/(1.0 + g));

	// set alphas
	m_LPF1.m_fAlpha = G;
	m_LPF2.m_fAlpha = G;
	m_HPF1.m_fAlpha = G;

	// set betas all are in the form of  <something>/((1 + g)
	m_LPF2.m_fBeta = (m_dK - m_dK*G)/(1.0 + g);
	m_HPF1.m_fBeta = -1.0/(1.0 + g);

	// set m_dAlpha0 variable
	m_dAlpha0 = 1.0/(1.0 - m_dK*G + m_dK*G*G); ;
}

double CKorgThreeFiveLPF::doFilter(double xn)
{
	// process input through LPF1
	double y1 = m_LPF1.doFilter(xn);

	// form feedback value
	double S35 = m_HPF1.getFeedbackOutput() + m_LPF2.getFeedbackOutput(); 
	
	// calculate u
	double u = m_dAlpha0*(y1 + S35);
	
	// NAIVE NLP
	if(m_uNonLinearProcessing == ON)
	{
		// Regular Version
		u = tanh(m_dSaturation*u);
	}

	// feed it to LPF2
	double y = m_dK*m_LPF2.doFilter(u);
	
	// feed y to HPF
	m_HPF1.doFilter(y);

	// auto-normalize
	if(m_dK > 0)
		y *= 1/m_dK;

	return y;
}

