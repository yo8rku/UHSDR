/*
 * mcp9801.c
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

/**
 * MCP9801 internal registers
 */
#define MCP_IIC_ADDR       		(0x90)

// MCP registers
#define MCP_TEMP        		(0x00)
#define MCP_CONFIG      		(0x01)
#define MCP_HYSTR       		(0x02)
#define MCP_LIMIT       		(0x03)

// MCP CONFIG register bits
#define MCP_ONE_SHOT    		(7)
#define MCP_ADC_RES     		(5)
#define MCP_FAULT_QUEUE 		(3)
#define MCP_ALERT_POL   		(2)
#define MCP_INT_MODE    		(1)
#define MCP_SHUTDOWN    		(0)
#define R_BIT           		(1)
#define W_BIT           		(0)

#define	MCP_ADC_RES_9			0
#define	MCP_ADC_RES_10			1
#define	MCP_ADC_RES_11			2
#define	MCP_ADC_RES_12			3

#define	MCP_POWER_UP			0
#define	MCP_POWER_DOWN			1

/**
 * MCP9801 driver working structure
 */
typedef struct {
	uint8_t addr;
} mcp9801_t;

static mcp9801_t mcp9801 = {
	.addr = MCP_IIC_ADDR,
};

/******************************************************************************
 * MCP9801 communication interface
 */

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
static int mcp9801_iic_read( uint8_t *dst, uint8_t ofs, uint8_t len)
{
	return mchf_hw_i2c_ReadBlock(mcp9801.addr, ofs, dst, len);
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
static int mcp9801_iic_write( uint8_t *src, uint8_t ofs, uint8_t len)
{
	return mchf_hw_i2c_WriteBlock( mcp9801.addr, ofs, src, len);
}

/******************************************************************************
 * Exported Functions to Access MCP9801
 *****************************************************************************/

/**
 * Read raw temperature value from MCP9801.
 * Temperature is returned in 4 digit offset decimal.
 * Example:
 * 12.25°C are returned as 122500.
 *
 * return temperature in 3 digit offset or -99 in case of error.
 */
int mcp9801_read( void)
{
	int ret;
	uint8_t data[2];

	/* Read temperature from chip */
	ret = mcp9801_iic_read( &data, MCP_TEMP, 2);
	if (ret)
		return -990000;

	/* Get unsigned degrees */
	ret = (int)(data[0] & 0x7f) * 10000;
	/* Add fractions */
	ret += ((int)data[1] * 625);
	/* Respect sign */
	if (data[0] & 0x80)
		ret *= -1;

	//printf("temp reg %02x%02x\n\r",data[0],data[1]);

	return ret;
}

/**
 * Initialize MCP9801 and verify communication
 *
 * return 0 on success or any error code
 */
int mcp9801_init(void)
{
	int ret;
	uint8_t config;

	/* Test communication and fetch configuration from chip */
	ret = mcp9801_iic_read( &config, MCP_CONFIG, 1);
	if(ret)
		return -1;

	//printf("chip conf: %02x\n\r",config);

	/* Configure needed resolution */
	config &= ~(3 << MCP_ADC_RES);
	config |= (MCP_ADC_RES_12 << MCP_ADC_RES);

	/* Wake chip from power-down */
	config &= ~(1 << MCP_SHUTDOWN);
	config |= (MCP_POWER_UP << MCP_SHUTDOWN);

	//printf("moded conf: %02x\n\r",config);

	/* Write configuration to MCP9801 */
	ret = mcp9801_iic_write( &config, MCP_CONFIG, 1);
	if(ret)
		return -1;

	//printf("updated conf: %02x\n\r",config);
	return 0;
}
