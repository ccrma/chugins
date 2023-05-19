// FIR Filter ChugIn, by Perry R. Cook, Version 1.0 October 2012

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR(FIR_ctor);
CK_DLL_DTOR(FIR_dtor);

CK_DLL_MFUN(FIR_setOrder);  // sets the order, allocates memory
CK_DLL_MFUN(FIR_getOrder);
CK_DLL_MFUN(FIR_setCoeff);  // sets an individual coefficient
CK_DLL_MFUN(FIR_getCoeff);

CK_DLL_MFUN(FIR_gaussian);  // creates a gaussian low pass with cutoff F
CK_DLL_MFUN(FIR_sinc);      // creates a sinc low pass with cutoff F

CK_DLL_MFUN(FIR_hpHetero);  // shift low pass up to this frequency
CK_DLL_MFUN(FIR_bpHetero);  // shift low pass to high pass (SR/2)

CK_DLL_TICK(FIR_tick);

t_CKINT FIR_data_offset = 0;

struct FIRData
{
    int order;          // filter order (number of multiplies, NUM_Delays+1)
    			// note a possible inconsistency here (delays vs. mults)
    float *coeff;	// filter coefficients
    float *buffer; 	// holds our delayed samples
};

CK_DLL_QUERY(FIR)
{
    QUERY->setname(QUERY, "FIR");
    
    QUERY->begin_class(QUERY, "FIR", "UGen");
    
    QUERY->add_ctor(QUERY, FIR_ctor);
    QUERY->add_dtor(QUERY, FIR_dtor);

    QUERY->doc_class(QUERY, "Yo!  This here is a ChugIn for ChucK. "
                     "It's a general-purpose FIR filter.\n"
                     "You make a new one:\n\n"
                     "FIR myFilter;\n\n"
                     "Then specify order:\n\n"
                     "N => myFilter.order;"
                     );
    QUERY->add_ex(QUERY, "filter/FIR-convolve-homer.ck");
    QUERY->add_ex(QUERY, "filter/FIR-greyhole-down-up-decimate.ck");
    QUERY->add_ex(QUERY, "filter/FIR-sinc-explicit.ck");

    QUERY->add_ugen_func(QUERY, FIR_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, FIR_setOrder, "int", "order");
    QUERY->add_arg(QUERY, "int", "arg");
    QUERY->doc_func(QUERY, "Set filter's order.");

    QUERY->add_mfun(QUERY, FIR_getOrder, "int", "order");
    QUERY->doc_func(QUERY, "Get filter's order.");

    QUERY->add_mfun(QUERY, FIR_setCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "idx");
    QUERY->add_arg(QUERY, "float", "coeff");
    QUERY->doc_func(QUERY, "Set filter idx's coefficient to coeff.");

    QUERY->add_mfun(QUERY, FIR_getCoeff, "float", "coeff");
    QUERY->add_arg(QUERY, "int", "idx");
    QUERY->doc_func(QUERY, "Set filter idx's coefficient.");

    QUERY->add_mfun(QUERY, FIR_gaussian, "int", "gaussian");
    QUERY->add_arg(QUERY, "float", "cutoff_freq");
    QUERY->doc_func(QUERY, "Create a gaussian lowpass filter with cutoff cutoff_freq.");

    QUERY->add_mfun(QUERY, FIR_sinc, "int", "sinc");
    QUERY->add_arg(QUERY, "float", "cutoff_freq");
    QUERY->doc_func(QUERY, "Create a sinc lowpass filter with cutoff cutoff_freq.");

    QUERY->add_mfun(QUERY, FIR_hpHetero, "int", "hpHetero");
    QUERY->doc_func(QUERY, ".");
    QUERY->doc_func(QUERY, "Cosine modulate lowpass filter to 1/2 the sample rate.");

    QUERY->add_mfun(QUERY, FIR_bpHetero, "int", "bpHetero");
    QUERY->add_arg(QUERY, "float", "freq");
    QUERY->doc_func(QUERY, "Cosine modulate lowpass filter to freq.");

    FIR_data_offset = QUERY->add_mvar(QUERY, "int", "@lpc_data", false);  // CHECK THIS!!
    
    QUERY->end_class(QUERY);

    return TRUE;
}

CK_DLL_CTOR(FIR_ctor)
{
    OBJ_MEMBER_INT(SELF, FIR_data_offset) = 0;
    
    FIRData * firdata = new FIRData;
    firdata->order = 4;   // defaults/placeholders here for constructor

    firdata->coeff = (float *) malloc(sizeof(float) * firdata->order);
    firdata->buffer = (float *) malloc(sizeof(float) * firdata->order);

    for (int i = 0; i < firdata->order; i++)  {
        firdata->coeff[i] = 0.25;		// default to moving average
        firdata->buffer[i] = 0.0;
    }

    OBJ_MEMBER_INT(SELF, FIR_data_offset) = (t_CKINT) firdata;
}

CK_DLL_DTOR(FIR_dtor)
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    if(firdata)
    {
	free(firdata->coeff);
	free(firdata->buffer);
        delete firdata;
        OBJ_MEMBER_INT(SELF, FIR_data_offset) = 0;
        firdata = NULL;
    }
}

CK_DLL_TICK(FIR_tick)
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    
    int i;

    *out = 0.0;

    for (i = 0; i < firdata->order; i++)  {    // run filter
        *out += firdata->coeff[i]*firdata->buffer[i];
    }
    	// for super huge orders, we should really use a circular buffer 
    for (i = firdata->order-1; i > 0; i--)  {  
        firdata->buffer[i] = firdata->buffer[i-1];
    }
    firdata->buffer[0] = in;

    return TRUE;
}

CK_DLL_MFUN(FIR_setOrder)
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    firdata->order = GET_NEXT_INT(ARGS);
    
    free(firdata->coeff);
    free(firdata->buffer);

    int i;

    firdata->coeff = (float *) malloc(sizeof(float) * firdata->order);
    firdata->buffer = (float *) malloc(sizeof(float) * firdata->order);
    float temp = 1.0 / firdata->order;
    for (i = 0; i < firdata->order; i++)  {
        firdata->coeff[i] = temp;  // default to moving average
        firdata->buffer[i] = 0.0;
    }

    RETURN->v_int = firdata->order;
}

CK_DLL_MFUN(FIR_getOrder)
{
    FIRData * FIRdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    RETURN->v_int = FIRdata->order;
}

CK_DLL_MFUN(FIR_setCoeff)  // form: blah.coeff(N,value);
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    int temp = GET_NEXT_INT(ARGS);
    float temp2 = GET_NEXT_FLOAT(ARGS);
    if (temp > firdata->order-1 || temp < 0) 
	printf("Illegal coefficient location! %i\n",temp);
    else firdata->coeff[temp] = temp2;
    RETURN->v_float = firdata->coeff[temp];
}

CK_DLL_MFUN(FIR_getCoeff)
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    RETURN->v_float = firdata->coeff[GET_NEXT_INT(ARGS)];
}

// useful functions to initialize our filter
void sincfill(int order, float *mycoeffs, float cutoff)   { // cutoff is fraction of SRATE/2
    int half = order / 2;
    int i;
    float phase;
    mycoeffs[half] = 1.0;
    for (i = 1; i < half; i++)  {
	phase = i*3.14159265359 / cutoff;
	mycoeffs[half+i] = sin(phase) / phase;
	mycoeffs[half-i] = mycoeffs[half+i];
    }
}

void hanning(int order, float * mycoeffs)  { // hanning window buffer, zero at ends
    int i;
    float phase = 1.0;
    for (i = 0; i < order; i++)  {
	mycoeffs[i] = mycoeffs[i] * 0.5 * (1.0 - cos(2.0*i*3.14159265359/order));
//	printf("%f ",mycoeffs[i]);  // SANITY CHECK ON GENERATED COEFFICIENTS
    }
//    printf("\n");
}

CK_DLL_MFUN(FIR_sinc)
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    float cutoff = GET_NEXT_FLOAT(ARGS);
    if (cutoff < 1.0)  {
	printf("Illegal sinc cutoff factor!!! (setting to 1.0)\n");
	cutoff = 1.0;
    }
    sincfill(firdata->order, firdata->coeff, cutoff);
    hanning(firdata->order,firdata->coeff);

    int i;
    float power = 0.0;
    for (i = 0; i < firdata->order; i++) power += firdata->coeff[i]*firdata->coeff[i];
    power = sqrt(power);
    for (i = 0; i < firdata->order; i++) firdata->coeff[i] = firdata->coeff[i] / power;
    printf("FIR: Sinc lowpass 1/%2.2f band (1/%2.2f SRate)\n",cutoff,2.0*cutoff);
}

CK_DLL_MFUN(FIR_gaussian)  {
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    float bandwidth = GET_NEXT_FLOAT(ARGS);

    if (bandwidth < 2.0) {
	printf("Silly bandwidth factor (setting to 2.0)\n");
	bandwidth = 2.0;
    }

    float temp = 6.28 / bandwidth / bandwidth;  // constant might be a hack!, but seems to work
    int exparg = firdata->order / 2;

    int i;
    float power = 0.0;
    for (i = 0; i < firdata->order; i++) {
	firdata->coeff[i] = exp(-exparg*exparg*temp);
	exparg = exparg -1;
    }
    hanning(firdata->order,firdata->coeff);
    for (i = 0; i < firdata->order; i++) power += firdata->coeff[i]*firdata->coeff[i];
    power = sqrt(power);
    for (i = 0; i < firdata->order; i++) firdata->coeff[i] = firdata->coeff[i] / power;
    printf("FIR: Gaussian(w/ hann) LP smoother with SRATE/%2.2f (3dB) bandwidth\n",2.0*bandwidth);
}

CK_DLL_MFUN(FIR_hpHetero) // cosine modulate LP filter to 1/2 sample rate
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    int i;
    float phase = 1.0;
    float power = 0.0;
    for (i = 0; i < firdata->order; i++) {
	firdata->coeff[i] = firdata->coeff[i] * phase;
	phase = phase * -1.0;
	power += firdata->coeff[i]*firdata->coeff[i];
    }
    power = sqrt(power);
    for (i = 0; i < firdata->order; i++) 
	firdata->coeff[i] = firdata->coeff[i] / power;
    printf("FIR: Complementing filter to 1/2 SRate (HP)\n");
}

CK_DLL_MFUN(FIR_bpHetero) // cosine modulate BP filter to this freq.
{
    FIRData * firdata = (FIRData *) OBJ_MEMBER_INT(SELF, FIR_data_offset);
    float centerFreq = GET_NEXT_FLOAT(ARGS);
    printf("FIR: Modulating filter to %f SRate (BP)\n",0.5*centerFreq);
    int i;
    float power = 0.0;
    centerFreq = 3.14159268 * centerFreq;
    for (i = 0; i < firdata->order; i++) {
	firdata->coeff[i] = firdata->coeff[i] * cos(i*centerFreq);
	power += firdata->coeff[i]*firdata->coeff[i];
    }
    power = sqrt(power);
    for (i = 0; i < firdata->order; i++) 
	firdata->coeff[i] = firdata->coeff[i] / power;
}
