 /******************************************************************************
 *
 * Module: PWM
 *
 * File Name: pwm.c
 *
 * Description: Source file for the PWM driver
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#include <avr/io.h>
#include "pwm.h"
#include "gpio.h"

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
/*
 * Description :
 * The function responsible for trigger the Timer0 with the PWM Mode.
 */
void PWM_Timer0_Start(uint8 duty_cycle)
{
	GPIO_setupPinDirection(PORTB_ID, PIN3_ID, PIN_OUTPUT); /* set PB3/OC0 as output pin --> pin where the PWM signal is generated from MC */

	TCNT0 = 0; /* Set Timer Initial Value */
	OCR0  = duty_cycle; /* Set Compare Value */

	/*
	 * Configure Timer0 Control Register:
	 * 1. Fast PWM mode FOC0=0
	 * 2. Fast PWM Mode WGM01=1 & WGM00=1
	 * 3. Clear OC0 when match occurs (non inverted mode) COM00=0 & COM01=1
	 * 4. clock = F_CPU/64 CS00=1 CS01=1 CS02=0
	 */
	TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01) | (1<<CS00);
}
