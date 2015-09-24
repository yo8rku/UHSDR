/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __UI_SI570
#define __UI_SI570

#define 	HIGHER_PRECISION
//#define 	LOWER_PRECISION

// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------

// The SI570 Min/Max frequencies are 4x the actual tuning frequencies
#define SI570_MIN_FREQ			  1000000	// 7.2 = 1.8 MHz
#define SI570_MAX_FREQ			200000000	// 128 = 32 Mhz
//
// These are "hard limit" frequencies below/above which the synthesizer/algorithms must not be adjusted or else the system may crash
#define SI570_HARD_MIN_FREQ		  1000000	// 1.0 = 0.25 MHz
#define SI570_HARD_MAX_FREQ		800000000	// 800 = 200 MHz

#define	LARGE_STEP_HYSTERESIS	10000//0.01		// size, in MHz, of hysteresis in large tuning step at output frequency (4x tuning freq)

#define SI570_RECALL			(1<<0)
#define SI570_FREEZE_DCO		(1<<4)
#define SI570_FREEZE_M			(1<<5)
#define SI570_NEW_FREQ			(1<<6)

#define SI570_REG_7				7
#define SI570_REG_135			135
#define SI570_REG_137			137

#define FACTORY_FXTAL			114.285

// VCO range
#define	FDCO_MAX 				5870
#define FDCO_MIN 	 			4650

#define POW_2_28          		268435456.0

// -------------------------------------------------------------------------------------
// Exports
// ------------------

int si570_get_configuration( void);
int lo_set_frequency( uint32_t freq, int calib, int temp_factor, int test);

int si570_init(void);

#endif
