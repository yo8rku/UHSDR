/*
 * lo.h
 *
 *  Created on: 24.09.2015
 *      Author: uprinz
 */

#ifndef DRIVERS_UI_OSCILLATOR_LO_H_
#define DRIVERS_UI_OSCILLATOR_LO_H_

/**
 * Local Oscillator Control
 */
int lo_set_frequency( uint32_t freq, int calib, int temp_factor, int test);
int lo_init(void);

#endif /* DRIVERS_UI_OSCILLATOR_LO_H_ */
