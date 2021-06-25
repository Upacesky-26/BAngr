/* ========================================
 *  XRegion - XRegion.h
 *  Copyright (c) 2016 airwindows, All rights reserved
 * ======================================== */

/* MIT License

  Copyright (c) 2018 Chris Johnson
  Copyright (C) 2021 Sven Jähnichen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include "XRegion.hpp"
#include <cstring>

XRegion::XRegion (const double rate) :
    rate (rate),
    params {0.5, 0.5, 0.5, 0.0, 1.0, 0.0}
{
	for (int x = 0; x < 15; x++) {biquad[x] = 0.0; biquadA[x] = 0.0; biquadB[x] = 0.0; biquadC[x] = 0.0; biquadD[x] = 0.0;}
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
}

XRegion::~XRegion() {}

void XRegion::process (float* input1, float* input2, float* output1, float* output2, int32_t sampleFrames)
{
    double gain = pow (params[0] + 0.5, 4);
	
	double high = params[1];
	double low = params[2];
	double mid = (high + low) * 0.5;
	double spread = 1.001 - fabs (high - low);
    double nuke = params[3];
	
	biquad[0] = high * high * high *20000.0 / rate;
	if (biquad[0] < 0.00009) biquad[0] = 0.00009;
	double compensation = sqrt(biquad[0])*6.4*spread;
	double clipFactor = 0.75+(biquad[0]*nuke*37.0);
	
    const double hm = 0.5 * (high + mid);
	biquadA[0] = hm * hm * hm *20000.0 / rate;
	if (biquadA[0] < 0.00009) biquadA[0] = 0.00009;
	double compensationA = sqrt(biquadA[0])*6.4*spread;
	double clipFactorA = 0.75+(biquadA[0]*nuke*37.0);
	
	biquadB[0] = mid * mid * mid *20000.0 / rate;
	if (biquadB[0] < 0.00009) biquadB[0] = 0.00009;
	double compensationB = sqrt(biquadB[0])*6.4*spread;
	double clipFactorB = 0.75+(biquadB[0]*nuke*37.0);
	
    const double ml = 0.5 * (mid + low);
	biquadC[0] = ml * ml * ml * 20000.0 / rate;
	if (biquadC[0] < 0.00009) biquadC[0] = 0.00009;
	double compensationC = sqrt(biquadC[0])*6.4*spread;
	double clipFactorC = 0.75+(biquadC[0]*nuke*37.0);
	
	biquadD[0] = low * low * low * 20000.0 / rate;
	if (biquadD[0] < 0.00009) biquadD[0] = 0.00009;
	double compensationD = sqrt(biquadD[0])*6.4*spread;
	double clipFactorD = 0.75+(biquadD[0]*nuke*37.0);
	
	double K = tan(M_PI * biquad[0]);
	double norm = 1.0 / (1.0 + K / 0.7071 + K * K);
	biquad[2] = K / 0.7071 * norm;
	biquad[4] = -biquad[2];
	biquad[5] = 2.0 * (K * K - 1.0) * norm;
	biquad[6] = (1.0 - K / 0.7071 + K * K) * norm;
	
	K = tan(M_PI * biquadA[0]);
	norm = 1.0 / (1.0 + K / 0.7071 + K * K);
	biquadA[2] = K / 0.7071 * norm;
	biquadA[4] = -biquadA[2];
	biquadA[5] = 2.0 * (K * K - 1.0) * norm;
	biquadA[6] = (1.0 - K / 0.7071 + K * K) * norm;
	
	K = tan(M_PI * biquadB[0]);
	norm = 1.0 / (1.0 + K / 0.7071 + K * K);
	biquadB[2] = K / 0.7071 * norm;
	biquadB[4] = -biquadB[2];
	biquadB[5] = 2.0 * (K * K - 1.0) * norm;
	biquadB[6] = (1.0 - K / 0.7071 + K * K) * norm;
	
	K = tan(M_PI * biquadC[0]);
	norm = 1.0 / (1.0 + K / 0.7071 + K * K);
	biquadC[2] = K / 0.7071 * norm;
	biquadC[4] = -biquadC[2];
	biquadC[5] = 2.0 * (K * K - 1.0) * norm;
	biquadC[6] = (1.0 - K / 0.7071 + K * K) * norm;
	
	K = tan(M_PI * biquadD[0]);
	norm = 1.0 / (1.0 + K / 0.7071 + K * K);
	biquadD[2] = K / 0.7071 * norm;
	biquadD[4] = -biquadD[2];
	biquadD[5] = 2.0 * (K * K - 1.0) * norm;
	biquadD[6] = (1.0 - K / 0.7071 + K * K) * norm;	
	
	double aWet = 1.0;
	double bWet = 1.0;
	double cWet = 1.0;
	double dWet = params[3] * 4.0;
	double wet = params[4];
    double pan = params[5];
	
	//four-stage wet/dry control using progressive stages that bypass when not engaged
	if (dWet < 1.0) {aWet = dWet; bWet = 0.0; cWet = 0.0; dWet = 0.0;}
	else if (dWet < 2.0) {bWet = dWet - 1.0; cWet = 0.0; dWet = 0.0;}
	else if (dWet < 3.0) {cWet = dWet - 2.0; dWet = 0.0;}
	else {dWet -= 3.0;}
	//this is one way to make a little set of dry/wet stages that are successively added to the
	//output as the control is turned up. Each one independently goes from 0-1 and stays at 1
	//beyond that point: this is a way to progressively add a 'black box' sound processing
	//which lets you fall through to simpler processing at lower settings.
	long double outSample = 0.0;
	
    while (--sampleFrames >= 0)
    {
		long double inputSampleL = *input1;
		long double inputSampleR = *input2;
		if (fabs(inputSampleL)<1.18e-37) inputSampleL = fpdL * 1.18e-37;
		if (fabs(inputSampleR)<1.18e-37) inputSampleR = fpdR * 1.18e-37;
		long double drySampleL = inputSampleL;
		long double drySampleR = inputSampleR;
		
		if (gain != 1.0) {
			inputSampleL *= gain;
			inputSampleR *= gain;
		}
		
		long double nukeLevelL = inputSampleL;
		long double nukeLevelR = inputSampleR;
		
		inputSampleL *= clipFactor;
		if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
		if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
		inputSampleL = sin(inputSampleL);
		outSample = biquad[2]*inputSampleL+biquad[4]*biquad[8]-biquad[5]*biquad[9]-biquad[6]*biquad[10];
		biquad[8] = biquad[7]; biquad[7] = inputSampleL; biquad[10] = biquad[9];
		biquad[9] = outSample; //DF1 left
		inputSampleL = outSample / compensation; nukeLevelL = inputSampleL;
		
		inputSampleR *= clipFactor;
		if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
		if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
		inputSampleR = sin(inputSampleR);
		outSample = biquad[2]*inputSampleR+biquad[4]*biquad[12]-biquad[5]*biquad[13]-biquad[6]*biquad[14];
		biquad[12] = biquad[11]; biquad[11] = inputSampleR; biquad[14] = biquad[13];
		biquad[13] = outSample; //DF1 right
		inputSampleR = outSample / compensation; nukeLevelR = inputSampleR;
		
		if (aWet > 0.0) {
			inputSampleL *= clipFactorA;
			if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
			if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
			inputSampleL = sin(inputSampleL);
			outSample = biquadA[2]*inputSampleL+biquadA[4]*biquadA[8]-biquadA[5]*biquadA[9]-biquadA[6]*biquadA[10];
			biquadA[8] = biquadA[7]; biquadA[7] = inputSampleL; biquadA[10] = biquadA[9];
			biquadA[9] = outSample; //DF1 left
			inputSampleL = outSample / compensationA; inputSampleL = (inputSampleL * aWet) + (nukeLevelL * (1.0-aWet));
			nukeLevelL = inputSampleL;
			
			inputSampleR *= clipFactorA;
			if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
			if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
			inputSampleR = sin(inputSampleR);
			outSample = biquadA[2]*inputSampleR+biquadA[4]*biquadA[12]-biquadA[5]*biquadA[13]-biquadA[6]*biquadA[14];
			biquadA[12] = biquadA[11]; biquadA[11] = inputSampleR; biquadA[14] = biquadA[13];
			biquadA[13] = outSample; //DF1 right
			inputSampleR = outSample / compensationA; inputSampleR = (inputSampleR * aWet) + (nukeLevelR * (1.0-aWet));
			nukeLevelR = inputSampleR;
		}
		if (bWet > 0.0) {
			inputSampleL *= clipFactorB;
			if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
			if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
			inputSampleL = sin(inputSampleL);
			outSample = biquadB[2]*inputSampleL+biquadB[4]*biquadB[8]-biquadB[5]*biquadB[9]-biquadB[6]*biquadB[10];
			biquadB[8] = biquadB[7]; biquadB[7] = inputSampleL; biquadB[10] = biquadB[9];
			biquadB[9] = outSample; //DF1 left
			inputSampleL = outSample / compensationB; inputSampleL = (inputSampleL * bWet) + (nukeLevelL * (1.0-bWet));
			nukeLevelL = inputSampleL;
			
			inputSampleR *= clipFactorB;
			if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
			if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
			inputSampleR = sin(inputSampleR);
			outSample = biquadB[2]*inputSampleR+biquadB[4]*biquadB[12]-biquadB[5]*biquadB[13]-biquadB[6]*biquadB[14];
			biquadB[12] = biquadB[11]; biquadB[11] = inputSampleR; biquadB[14] = biquadB[13];
			biquadB[13] = outSample; //DF1 right
			inputSampleR = outSample / compensationB; inputSampleR = (inputSampleR * bWet) + (nukeLevelR * (1.0-bWet));
			nukeLevelR = inputSampleR;
		}
		if (cWet > 0.0) {
			inputSampleL *= clipFactorC;
			if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
			if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
			inputSampleL = sin(inputSampleL);
			outSample = biquadC[2]*inputSampleL+biquadC[4]*biquadC[8]-biquadC[5]*biquadC[9]-biquadC[6]*biquadC[10];
			biquadC[8] = biquadC[7]; biquadC[7] = inputSampleL; biquadC[10] = biquadC[9];
			biquadC[9] = outSample; //DF1 left
			inputSampleL = outSample / compensationC; inputSampleL = (inputSampleL * cWet) + (nukeLevelL * (1.0-cWet));
			nukeLevelL = inputSampleL;
			
			inputSampleR *= clipFactorC;
			if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
			if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
			inputSampleR = sin(inputSampleR);
			outSample = biquadC[2]*inputSampleR+biquadC[4]*biquadC[12]-biquadC[5]*biquadC[13]-biquadC[6]*biquadC[14];
			biquadC[12] = biquadC[11]; biquadC[11] = inputSampleR; biquadC[14] = biquadC[13];
			biquadC[13] = outSample; //DF1 right
			inputSampleR = outSample / compensationC; inputSampleR = (inputSampleR * cWet) + (nukeLevelR * (1.0-cWet));
			nukeLevelR = inputSampleR;
		}
		if (dWet > 0.0) {
			inputSampleL *= clipFactorD;
			if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
			if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
			inputSampleL = sin(inputSampleL);
			outSample = biquadD[2]*inputSampleL+biquadD[4]*biquadD[8]-biquadD[5]*biquadD[9]-biquadD[6]*biquadD[10];
			biquadD[8] = biquadD[7]; biquadD[7] = inputSampleL; biquadD[10] = biquadD[9];
			biquadD[9] = outSample; //DF1 left
			inputSampleL = outSample / compensationD; inputSampleL = (inputSampleL * dWet) + (nukeLevelL * (1.0-dWet));
			nukeLevelL = inputSampleL;
			
			inputSampleR *= clipFactorD;
			if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
			if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
			inputSampleR = sin(inputSampleR);
			outSample = biquadD[2]*inputSampleR+biquadD[4]*biquadD[12]-biquadD[5]*biquadD[13]-biquadD[6]*biquadD[14];
			biquadD[12] = biquadD[11]; biquadD[11] = inputSampleR; biquadD[14] = biquadD[13];
			biquadD[13] = outSample; //DF1 right
			inputSampleR = outSample / compensationD; inputSampleR = (inputSampleR * dWet) + (nukeLevelR * (1.0-dWet));
			nukeLevelR = inputSampleR;
		}
		
		if (inputSampleL > 1.57079633) inputSampleL = 1.57079633;
		if (inputSampleL < -1.57079633) inputSampleL = -1.57079633;
		inputSampleL = sin(inputSampleL);
		if (inputSampleR > 1.57079633) inputSampleR = 1.57079633;
		if (inputSampleR < -1.57079633) inputSampleR = -1.57079633;
		inputSampleR = sin(inputSampleR);
		
		if (wet < 1.0) {
			inputSampleL = (drySampleL * (1.0-wet))+(inputSampleL * wet);
			inputSampleR = (drySampleR * (1.0-wet))+(inputSampleR * wet);
		}
		
		//begin 32 bit stereo floating point dither
		int expon; frexpf((float)inputSampleL, &expon);
		fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
		inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 5.5e-36l * (1 << (expon+62)));
		frexpf((float)inputSampleR, &expon);
		fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
		inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 5.5e-36l * (1 << (expon+62)));
		//end 32 bit stereo floating point dither
		
		*output1 = inputSampleL * (1.0f - (pan > 0.0f) * pan);
		*output2 = inputSampleR * (1.0f + (pan < 0.0f) * pan);

		input1++;
		input2++;
		output1++;
		output2++;
    }
}

void XRegion::setParameters (const float* values) 
{
    memcpy (params, values, 6 * sizeof (float));
}

float* XRegion::getParameters () {return params;}
