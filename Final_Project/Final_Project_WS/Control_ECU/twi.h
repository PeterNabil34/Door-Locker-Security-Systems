 /******************************************************************************
 *
 * Module: TWI(I2C)
 *
 * File Name: twi.h
 *
 * Description: Header file for the TWI(I2C) AVR driver
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/ 

#ifndef TWI_H_
#define TWI_H_

#include "std_types.h"

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

typedef uint8 TWI_Address;
typedef uint8 TWI_BaudRate;

typedef enum
{
	ONE, FOUR, SIXTEEN, SIXTY_FOUR
}TWI_Prescaler;

typedef struct
{
	TWI_Address address;
	TWI_BaudRate bit_rate;
	TWI_Prescaler prescaler;
}TWI_ConfigType;

/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/

/* I2C Status Bits in the TWSR Register */
#define TWI_START         0x08 /* start has been sent */
#define TWI_REP_START     0x10 /* repeated start */
#define TWI_MT_SLA_W_ACK  0x18 /* Master transmit ( slave address + Write request ) to slave + ACK received from slave. */
#define TWI_MT_SLA_R_ACK  0x40 /* Master transmit ( slave address + Read request ) to slave + ACK received from slave. */
#define TWI_MT_DATA_ACK   0x28 /* Master transmit data and ACK has been received from Slave. */
#define TWI_MR_DATA_ACK   0x50 /* Master received data and send ACK to slave. */
#define TWI_MR_DATA_NACK  0x58 /* Master received data but doesn't send ACK to slave. */

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Setup the Prescaler, Slave Address, the Baud Rate and enable the TWI(I2C) Module.
 */
void TWI_init(const TWI_ConfigType * Config_Ptr);

/*
 * Description :
 * Functional responsible for Sending a Start-Bit.
 */
void TWI_start(void);

/*
 * Description :
 * Functional responsible for Sending a Stop-Bit.
 */
void TWI_stop(void);

/*
 * Description :
 * Functional responsible for Writing a byte of data by TWI.
 */
void TWI_writeByte(uint8 data);

/*
 * Description :
 * Functional responsible for Reading a byte of data followed by an Acknowledgment by TWI.
 */
uint8 TWI_readByteWithACK(void);

/*
 * Description :
 * Functional responsible for Reading a byte of data followed by an Non-Acknowledgment by TWI.
 */
uint8 TWI_readByteWithNACK(void);

/*
 * Description :
 * Functional responsible for Reading the TWI Status Register.
 */
uint8 TWI_getStatus(void);


#endif /* TWI_H_ */
