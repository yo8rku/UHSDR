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
**                                                                                 **
**  Heavily modified and optimized by Astralix on inspirations from DF8OE		   **
************************************************************************************/

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "mchf_board.h"		/**> Always add this first for configuration */
#include "mchf_hw_i2c.h"

#include "si570.h"

typedef struct {
	uint8_t hs_div;		//> Reg  7: 7..5: HS_DIV[2:0] | 4..0: N1[6:2]
	uint8_t ref_fq;		//> Reg  6: 7..6: N1[1:0] | RFREQ[37..32]
	// TODO: try to whack this in in bits and pieces...
} si570_regs_t;

typedef struct __attribute__ ((packed)) {
	uint8_t addr;		//> IIC bus address of this chip (0x50 or 0x55)
	uint8_t regs[6];	//> Mirror of SI570 internal registers
} si570_t;

static si570_t si570 = {

};

const uchar 	hs_div[6]	= {11, 9, 7, 6, 5, 4};
const float 	fdco_max 	= FDCO_MAX;
const float 	fdco_min 	= FDCO_MIN;

// all known startup frequencies
float suf_table[] = {
		 10.0000,  10.3560,  14.0500,  14.1000,
		 15.0000,  16.0915,  22.5792,  34.2850,
		 56.3200,  63.0000,  76.8000, 100.0000,
		125.0000, 156.2500,
		0
};

typedef struct OscillatorState
{
	long double		rfreq;
	long double 	rfreq_old;

	float 			fxtal;

	uchar			regs[6];

	float			fout;		// contains startup frequency info of Si570

#ifdef LOWER_PRECISION
	uchar 			init_n1;
	uchar			init_hsdiv;
	ulong 			init_rfreq;
#endif

} OscillatorState;


// All publics as struct, so eventually could be malloc-ed and in CCM for faster access!!
__IO OscillatorState os;

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_setbits
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_setbits(unsigned char original, unsigned char reset_mask, unsigned char new_val)
{
   	return ((original & reset_mask) | new_val);
}

/**
 * Read one or more bytes from SI570 chip.
 *
 * dst: pointer to write fetched bytes to.
 * ofs: offset address in chips' registers.
 * len: number of bytes to read.
 *
 * return: 0 on success, else -1
 *
 * remarks:
 * Optimize IIC routines to support auto-increment addressing if chip supports it.
 * Extend error return codes to be more detailed.
 */
static int si570_iic_read( uint8_t *dst, uint8_t ofs, uint8_t len)
{
	return mchf_hw_i2c_ReadBlock(si570.addr, ofs, dst, len);
}

/**
 * Write one or more bytes to SI570 chip.
 *
 * src: pointer to fetch bytes from.
 * ofs: offset start address in chips' registers.
 * len: number of bytes to write.
 *
 * return: 0 on success, else -1
 *
 * remarks:
 * Optimize IIC routines to support auto-increment addressing if chip supports it.
 * Extend error return codes to be more detailed.
 */
static int si570_iic_write( uint8_t *src, uint8_t ofs, uint8_t len)
{
	return mchf_hw_i2c_WriteBlock( si570.addr, ofs, src, len);
}

/**
 * Detect SI570 IIC address.
 *
 * return: 0 on success, -1 if not found.
 */
static int si570_detect( void) {
	int ret;

	/* Try primary address */
	si570.addr = (0x50 << 1);
	ret = si570_iic_read( si570.regs, SI570_REG_7, 1);
	if (!ret)
		return ret;

	/* Try secondary address */
	si570.addr = (0x55 << 1);
	ret = si570_iic_read( si570.regs, SI570_REG_7, 1);

	return ret;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_verify_frequency
//* Object              : read regs and verify freq change
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/**
 * Verify settings of SI570 against buffered settings in working structure.
 *
 * return: -1 in case of mismatch, else 0.
 */
static int ui_si570_verify_frequency(void)
{
	int ret = 0;
	uint8_t regs[10];

	/* Read frequency related registers */
	ret = si570_iic_read(&regs[0], SI570_REG_7, 6);
	if (ret)
		return ret;

	// Not working - need fix
	//memset(regs,0,6);
	//trx4m_hw_i2c_ReadData(si570_address,7,regs,5);

	//printf("-----------------------------------\n\r");
	//printf("write %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);
	//printf("read  %02x %02x %02x %02x %02x %02x\n\r",regs[0],regs[1],regs[2],regs[3],regs[4],regs[5]);

	/* Compare read and buffered data, return -1 in case of mismatch */
	ret = memcmp( regs, si570.regs, 6);
	if (ret)
		return -1;

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_small_frequency_change
//* Object              : small frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static int ui_si570_small_frequency_change(void)
{
	int ret;
	uint8_t r135;

	//printf("small\n\r");

	/* Read status of SI570 */
	ret = si570_iic_read( &r135, SI570_REG_135, 1);
	if(ret)
		return -1;

	/* Set "Freeze M" bit */
	r135 |= SI570_FREEZE_M;
	ret = si570_iic_write( &r135, SI570_REG_135, 1);
	if(ret)
		return -1;

	/* Update chip with new frequency in registers 7 to 12 */
	ret = si570_iic_write( si570.regs, SI570_REG_7, 6);
	if(ret)
		goto error_unfreeze;

	/* Verify settings again */
	// TODO: If IIC works reliable, this is just wasting code and time
	ret = ui_si570_verify_frequency();
	if(ret)
		goto error_unfreeze;

	/* Un-freeze SI570 */
	r135 &= ~SI570_FREEZE_M;
	ret = si570_iic_write( &r135, SI570_REG_135, 1);

	return 0;

error_unfreeze:
	r135 &= ~SI570_FREEZE_M;
	ret = si570_iic_write( &r135, SI570_REG_135, 1);
	return -1;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_large_frequency_change
//* Object              : large frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static int ui_si570_large_frequency_change(void)
{
	int ret;
	uint8_t r135, r137;
	uint8_t unfreeze = 0;

	//printf("large\n\r");

	// Read the current state of Register 137
	// TODO: Why? There is only one bit and we need to write it to 1
	// ret = si570_read( &r137, SI570_REG_137, 1);
	// if(ret)
	//	return -1;

	/* Set the Freeze DCO bit */
	r137 = SI570_FREEZE_DCO;
	ret = si570_iic_write( &r137, SI570_REG_137, 1);
	if(ret)
		return -1;

	/* Write new frequency as block */
	ret = si570_iic_write( si570.regs, SI570_REG_7, 6);
	if(ret)
		goto error_unfreeze;

	/* Verify... */
	// TODO: Skip that!
	ret = ui_si570_verify_frequency();
	if(ret)
		goto error_unfreeze;

	/* Release DCO bit */
	r137 &= ~SI570_FREEZE_DCO;
	ret = si570_iic_write( &r137, SI570_REG_137, 1);
	if(ret)
		return -1;

	/* Read current status of SI570 and set NewFreq bit in it to cause
	 * the SI570 to update its oscillators...
	 */
	ret = si570_iic_read( &r135, SI570_REG_135, 1);
	if(ret) return -1;

	r135 |= SI570_NEW_FREQ;
	ret = si570_iic_write( &r135, SI570_REG_135, 1);
	if(ret) return -1;

	/* Loop here until Frequencies are stable and status reports locking */
	while(r135 & SI570_NEW_FREQ)
	{
		// TODO: This is probably a good point to add some delay instead of DDOS attacking the IIC
		ret = si570_iic_read( &r135, SI570_REG_135, 1);
		if(ret)
			return -1;
	}

	return 0;

error_unfreeze:
	r137 &= ~SI570_FREEZE_DCO;
	ret = si570_iic_write( &r135, SI570_REG_137, 1);
	return -1;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_is_large_change
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_is_large_change(void)
{
	long double	delta_rfreq;

	if(os.rfreq_old == os.rfreq)
		return 0;

	if(os.rfreq_old < os.rfreq)
		delta_rfreq = os.rfreq - os.rfreq_old;
	else
		delta_rfreq = os.rfreq_old - os.rfreq;

	if((delta_rfreq / os.rfreq_old) <= 0.0035)
		return 0;

	return 1;
}
	
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_get_configuration
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
int ui_si570_get_configuration(void)
{
   	int 	ret;

   	uint8_t r135;
   	int		timeout;

   	int 	i;
   	short	res;

   	ulong 	rfreq_frac;
   	ulong 	rfreq_int;

   	uchar 	hsdiv_curr;
   	uchar 	n1_curr;
   	//uchar	sig[10];

   	// Reset publics
   	os.rfreq_old 	= 0.0;
   	os.fxtal 		= FACTORY_FXTAL;

   	/* Set RECALL bit in SI570 to fetch values from it
   	 * WARNING:
   	 * This resets the SI570 into its factory defaults!
   	 * logically it is an internal reset without aborting IIC communication.
   	 */
   	r135 = SI570_RECALL;
   	ret = si570_iic_write( &r135, SI570_REG_135, 1);
   	if (ret)
   		return -1;

   	/* Now wait until this bit is cleard by internal logic */
	ret = SI570_RECALL;
	timeout = 30;
	while( timeout-- && (ret & SI570_RECALL)) {
		ret = si570_iic_read( &r135, SI570_REG_135, 1);
		if (ret)
			return -2;
	}
	if (timeout == 0) // dead communication
		return -3;

	/* Fetch factory settings from chip */
	ret = si570_iic_read( si570.regs, SI570_REG_7, 6);
	if (ret)
		return -4;

	//printf("startup %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);

   	hsdiv_curr = ((os.regs[0] & 0xE0) >> 5) + 4;

#ifdef LOWER_PRECISION
   	os.init_hsdiv = hsdiv_curr;
   	//printf("init hsdiv: %d\n\r",hsdiv_curr);
#endif

   	n1_curr = ((os.regs[0] & 0x1F ) << 2 ) + ((os.regs[1] & 0xC0 ) >> 6 );
   	if(n1_curr == 0)
   		n1_curr = 1;
   	else if((n1_curr & 1) != 0)
   		n1_curr = n1_curr + 1;

#ifdef LOWER_PRECISION
   	os.init_n1 = n1_curr;
   	//printf("init n1: %d\n\r",n1_curr);
#endif

 	rfreq_int =	(os.regs[1] & 0x3F);
  	rfreq_int =	(rfreq_int << 4) + ((os.regs[2] & 0xF0) >> 4);

	rfreq_frac = (os.regs[2] & 0x0F);
  	rfreq_frac = (rfreq_frac << 8) + os.regs[3];
  	rfreq_frac = (rfreq_frac << 8) + os.regs[4];
  	rfreq_frac = (rfreq_frac << 8) + os.regs[5];

#ifdef LOWER_PRECISION
  	os.init_rfreq = (os.regs[1] & 0x3F );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[2] );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[3] );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[4] );
  	os.init_rfreq = (os.init_rfreq << 6) + (os.regs[5] >> 2 );
#endif

  	os.rfreq = rfreq_int + rfreq_frac / POW_2_28;
  	os.fxtal = (os.fout * n1_curr * hsdiv_curr) / os.rfreq;
	
  	// Read signature
  	/*for(i = 0; i < 6; i++)
  	{
   		res = mchf_hw_i2c_ReadRegister(si570_address,(i + 13) ,&sig[i]);
   		if(res != 0)
   		{
			printf("read sig err: %d, i = %d\n\r",res,i);
			return 4;
		}
   	}
   	printf("sig %02x %02x %02x %02x %02x %02x\n\r",sig[0],sig[1],sig[2],sig[3],sig[4],sig[5]);*/

	return 0;
}

/**
 * Calculate the FXTAL that deviates from part to part
 * Read SILABs datasheet FAQ Nr. 3 and section 3.2
 *
 * To have this base frequency at hand for all further calculations,
 * it will be fetched and simplified here as needed.
 */
static void si570_calc_startupfrequency(void)
{
	if (os.fout < 5)
	{
		uint8_t regs[7];
		uint8_t hs_div;
		uint8_t n1;
		uint64_t rsfreq;

		// float rsfreq;

		/* Settings have been read at previous parts of init sequence,
		 * so assemble settings and frequencies from single bytes...
		 */
		rsfreq = (regs[5] << 0) | (regs[4] << 8) | (regs[3] << 16) | (regs[2] << 24);
		rsfreq |= (regs[1] & 0x3F) << 32;
		rsfreq *= POW_2_28;

		// / (double));

		hs_div = (regs[0] >> 5) + 4;
		n1 = (regs[1] >> 6) | ((regs[0] & 0x1F) << 2) + 1;

		if ((n1 & 1) && (n1 != 1))
			n1++;

		os.fout = roundf((1142850 * rsfreq) / (hs_div * n1)) / 10000;

		int i;

		// test if startup frequency is known
		for (i = 0; suf_table[i] != 0; i++)
		{
			float test = os.fout - suf_table[i];
			if( test < 0)
				test = -test;
			if(test < 0.2)
			{
				os.fout = suf_table[i];
				break;
			}
		}
	}
}

/**
 * Initialize SI570
 *
 * Detect SI570 address and type, attach it to the system.
 * Preset initial frequency settings and control bits.
 */
int si570_init( void)
{
	int ret;

	/* Detect SI570 at IIC bus */
	ret = si570_detect();
	if (ret)
		return ret;

	/* Initially fetch all settings */
	ret = si570_iic_read( si570.regs, SI570_REG_7, 6);

	/* Detect the different frequency types of this chips */
	si570_calc_startupfrequency();

}
