/*
 * mcp9801.h
 *
 *  Created on: 24.09.2015
 *      Author: uprinz
 */

#ifndef DRIVERS_UI_OSCILLATOR_MCP9801_H_
#define DRIVERS_UI_OSCILLATOR_MCP9801_H_

/* Exported functions */
int si570_read_temp( int *temp);
int si570_conv_temp( uchar *temp, int *dtemp);

int mcp9801_init(void);

#endif /* DRIVERS_UI_OSCILLATOR_MCP9801_H_ */
