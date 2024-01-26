 /******************************************************************************
 *
 * Module: Timer1
 *
 * File Name: timer1.c
 *
 * Description: Source file for the Timer1 driver
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1.h"
#include "gpio.h"
#include "common_macros.h"

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

/* Global variables to hold the address of the call back function */
static volatile void (*g_callBackPtr)(void) = NULL_PTR;

/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/

#ifdef NORMAL_MODE
ISR(TIMER1_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		(*g_callBackPtr)();
	}
}
#endif

#ifdef COMPARE_MODE_A
ISR(TIMER1_COMPA_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		(*g_callBackPtr)();
	}
}
#endif

#ifdef COMPARE_MODE_B
ISR(TIMER1_COMPB_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		(*g_callBackPtr)();
	}
}
#endif

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description :
 *
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr)
{
	TCCR1A = (TCCR1A & 0xFC) | ((Config_Ptr->mode) & 0x03);

#ifdef PWM_MODE_A
	CLEAR_BIT(TCCR1A,FOC1A);
#else
	SET_BIT(TCCR1A,FOC1A);
#endif

#ifdef PWM_MODE_B
	CLEAR_BIT(TCCR1A,FOC1B);
#else
	SET_BIT(TCCR1A,FOC1B);
#endif

#ifdef COMPARE_OUTPUT_MODE_A
	TCCR1A |= (TCCR1A & 0x3F) | (COM1A << 6);
	GPIO_setupPinDirection(PORTD_ID, PIN5_ID, PIN_OUTPUT); /* OC1A */
	GPIO_writePin(PORTD_ID, PIN5_ID, LOGIC_LOW);
#endif

#ifdef COMPARE_OUTPUT_MODE_B
	TCCR1A |= (TCCR1A & 0xCF) | (COM1B << 4);
	GPIO_setupPinDirection(PORTD_ID, PIN4_ID, PIN_OUTPUT); /* OC1B */
	GPIO_writePin(PORTD_ID, PIN4_ID, LOGIC_LOW);
#endif

	TCCR1B = (TCCR1B & 0xE7) | (((Config_Ptr->mode) & 0x0C) << 1) | (TCCR1B & 0xF8) | (Config_Ptr->prescaler);
	TCNT1 = (Config_Ptr->initial_value);

#ifdef NORMAL_MODE
	TIMSK |= (1<<TOIE1);
#endif

#ifdef COMPARE_MODE_A
	OCR1A = (Config_Ptr->compare_value);
	TIMSK |= (1<<OCIE1A);
#endif

#ifdef COMPARE_MODE_B
	OCR1B = (Config_Ptr->compare_value);
	TIMSK |= (1<<OCIE1B);
#endif
}

/*
 * Description : The function is responsible for clear all Timer1/ICU registers,
 * disable all its interrupts and reset the global pointer value to NULL.
 *
 */
void Timer1_deInit(void)
{
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	ICR1 = 0;
	OCR1A = 0;
	OCR1B = 0;
	CLEAR_BIT(TIMSK, TOIE1);
	CLEAR_BIT(TIMSK, OCIE1A);
	CLEAR_BIT(TIMSK, OCIE1B);
	CLEAR_BIT(TIMSK, TICIE1);
	g_callBackPtr = NULL_PTR;
}

/*
 * Description : The function is responsible for saving the address
 * of the call-back function in a global variable (pointer to function).
 *
 */
void Timer1_setCallBack(void(*a_ptr)(void))
{
	g_callBackPtr = a_ptr;
}

