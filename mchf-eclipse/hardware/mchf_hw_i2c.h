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

#ifndef __MCHF_HW_I2C_H
#define __MCHF_HW_I2C_H

void mchf_hw_i2c_init(void);
void mchf_hw_i2c_reset(void);

int mchf_hw_i2c_WriteRegister( uint8_t I2CAddr, uint8_t RegisterAddr, uint8_t *data);
int mchf_hw_i2c_WriteBlock( uint8_t I2CAddr, uint8_t RegisterAddr, uint8_t *data, uint8_t size);

int mchf_hw_i2c_ReadRegister( uint8_t I2CAddr,uint8_t RegisterAddr, uint8_t *data);
int mchf_hw_i2c_ReadBlock( uint8_t I2CAddr,uint8_t RegisterAddr, uint8_t *data, uint8_t size);

#endif
