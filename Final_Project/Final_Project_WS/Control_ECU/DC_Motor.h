 /******************************************************************************
 *
 * Module: DC-Motor
 *
 * File Name: DC_Motor.h
 *
 * Description: Header file for the DC-Motor driver
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define DC_MOTOR_FIRST_PORT                PORTC_ID
#define DC_MOTOR_FIRST_PIN                 PIN2_ID

#define DC_MOTOR_SECOND_PORT               PORTC_ID
#define DC_MOTOR_SECOND_PIN                PIN3_ID

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/
typedef enum
{
	STOP,CLOCKWISE,ANTI_CLOCKWISE
}DcMotor_State;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
/*
 * Description :
 * Function responsible for setup the direction for the two motor pins and stop at the DC-Motor at the beginning.
 */
void DcMotor_Init(void);

/*
 * Description :
 * Function responsible for rotate the DC Motor CW/ or A-CW or stop the motor based on the state input state value
 * and send the required duty cycle to the PWM driver based on the required speed value.
 */
void DcMotor_Rotate(DcMotor_State state,uint8 speed);

#endif /* DC_MOTOR_H_ */
