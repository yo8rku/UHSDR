/*
 * lo.c
 *
 *  Created on: 24.09.2015
 *      Author: uprinz
 */

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "mchf_board.h"		/**> Always add this first for configuration */
#include "mchf_hw_i2c.h"

#include "mcp9801.h"
#include "si570.h"

#include "lo.h"

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_change_frequency
//* Object              :
//* Input Parameters    : input frequency (float), test: 0 = tune, 1 = calculate, but do not actually tune to see if a large tuning step will occur
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/**
 * Set LO frequency
 *
 */
static int lo_change_frequency( float new_freq, int test)
{
	uchar 	i,res = 0;
   	ushort 	divider_max,curr_div,whole;

   	float 	curr_n1;
   	float 	n1_tmp;

   	ulong 	frac_bits;

#ifdef LOWER_PRECISION
   	float 	ratio = 0;
   	ulong 	final_rfreq_long;
#endif

   	uchar 	n1;
   	uchar 	hsdiv;

   	divider_max = (ushort)floorf(fdco_max / new_freq);
   	curr_div 	= (ushort)ceilf (fdco_min / new_freq);
   	//printf("%d-%d -> ",curr_div,divider_max);

   	while (curr_div <= divider_max)
	{
    	for(i = 0; i < 6; i++)
      	{
    		hsdiv = hs_div[i];
      		curr_n1 = (float)(curr_div) / (float)(hsdiv);

      		n1_tmp = floorf(curr_n1);
      		n1_tmp = curr_n1 - n1_tmp;

      		if(n1_tmp == 0.0)
        	{
         		n1 = (uchar)curr_n1;
				if((n1 == 1) || ((n1 & 1) == 0))
					goto found;
         	}
      	}

      	curr_div++;
   	}

   	CriticalError(102);

found:

	//printf("(%d) %d %d\n\r",curr_div,n1,hsdiv);

 	// New RFREQ calculation
	os.rfreq = ((long double)new_freq * (long double)(n1 * hsdiv)) / os.fxtal;
   	//printf("%d\n\r",(int)os.rfreq);

   	// Debug print calc freq
   	//printf("%d\n\r",(int)((os.fxtal*os.rfreq)/(n1*hsdiv)));

#ifdef LOWER_PRECISION
   	ratio = new_freq / fout0;
   	ratio = ratio * (((float)n1)/((float)os.init_n1));
   	ratio = ratio * (((float)hsdiv)/((float)os.init_hsdiv));
   	final_rfreq_long = ratio * os.init_rfreq;
#endif

   	for(i = 0; i < 6; i++)
   		os.regs[i] = 0;

   	hsdiv = hsdiv - 4;
   	os.regs[0] = (hsdiv << 5);

   	if(n1 == 1)
     	n1 = 0;
   	else if((n1 & 1) == 0)
      	n1 = n1 - 1;

   	os.regs[0] = ui_si570_setbits(os.regs[0], 0xE0, (n1 >> 2));
   	os.regs[1] = (n1 & 3) << 6;

#ifdef LOWER_PRECISION
   	os.regs[1] = os.regs[1] | (final_rfreq_long >> 30);
   	os.regs[2] = final_rfreq_long >> 22;
   	os.regs[3] = final_rfreq_long >> 14;
   	os.regs[4] = final_rfreq_long >> 6;
   	os.regs[5] = final_rfreq_long << 2;
#endif

#ifdef HIGHER_PRECISION
   	whole = floorf(os.rfreq);
   	frac_bits = floorf((os.rfreq - whole) * POW_2_28);

   	for(i = 5; i >= 3; i--)
   	{
   		os.regs[i] = frac_bits & 0xFF;
   		frac_bits = frac_bits >> 8;
   	}

   	os.regs[2] = ui_si570_setbits(os.regs[2], 0xF0, (frac_bits & 0xF));
   	os.regs[2] = ui_si570_setbits(os.regs[2], 0x0F, (whole & 0xF) << 4);
   	os.regs[1] = ui_si570_setbits(os.regs[1], 0xC0, (whole >> 4) & 0x3F);
#endif

   	//printf("write %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);

   	// check to see if this tuning will result in a "large" tuning step, without setting the frequency
   	if(test)
   		return( si570_is_large_change());

	if( si570_is_large_change()) {
		res = si570_large_frequency_change();
	} else {
		res = si570_small_frequency_change();

		// Maybe large change was needed, let's try it
		if(res)
			res = si570_large_frequency_change();
	}

	if(res)
		return res;

	// Verify second time - we might be transmitting, so
	// it is absolutely unacceptable to be on startup
	// SI570 frequency if any I2C error or chip reset occurs!
	res = ui_si570_verify_frequency();
	if(res == 0)
		os.rfreq_old = os.rfreq;

	//if(res)
	//	printf("---- error ----\n\r");

	return res;
}

/**
 * Set new frequency to SI570
 *
 * freq:	frequency in Hz (32 bits)
 * calib:	calibration factor to use
 * temp_factor:	temperature calibration factor (referenced to 14.000MHz)
 * test:	0: set frequency, 1: calculate but do not write.
 */
int lo_set_frequency( uint32_t freq, int calib, int temp_factor, int test)
{
	long double d;
	float		si_freq, freq_calc, freq_scale, temp_scale, temp;

	freq_scale = (float)freq;	// get frequency
	freq_calc = freq_scale;		// copy frequency
	freq_scale /= 14000000;		// get scaling factor since our calibrations are referenced to 14.000 MHz

	temp = (float)calib;		// get calibration factor
	temp *= (freq_scale);		// scale calibration for operating frequency but double magnitude of calibration factor
	freq_calc -= temp;			// subtract calibration factor

	temp_scale = (float)temp_factor;	// get temperature factor
	temp_scale /= 14000000;		// calculate scaling factor for the temperature correction (referenced to 14.000 MHz)

	freq_calc *= (1 + temp_scale);	// rescale by temperature correction factor

	freq = (ulong)freq_calc;

	// -----------------------------
	// freq = freq + calib*4 + temp_factor;		// This is the old version - it does NOT scale calibration/temperature with operating frequency.
	// -----------------------------

	d = freq;								// convert to float
	d = d / 1000000.0;						// Si570 set value = decimal MHz
	si_freq = d;							// convert to float

	//printf("set si750 freq to: %d\n\r",freq);

	return ui_si570_change_frequency(si_freq, test);
}

int lo_set_frequency( uint32_t freq, int calib, int temp_factor, int test)
{

}

int lo_init(void)
{

}

