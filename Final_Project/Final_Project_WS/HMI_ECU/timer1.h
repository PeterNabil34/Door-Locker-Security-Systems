 /******************************************************************************
 *
 * Module: Timer1
 *
 * File Name: timer1.h
 *
 * Description: Header file for the Timer1 driver
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#ifndef TIMER1_H_
#define TIMER1_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define TOP 65535

/* The programmer has to uncomment at least one of these 3 #defines (NORMAL_MODE, COMPARE_MODE_A & COMPARE_MODE_B) */
/* #define NORMAL_MODE */
#define COMPARE_MODE_A
/* #define COMPARE_MODE_B */

/* #define PWM_MODE_A */
/* #define PWM_MODE_B */
/* #define COMPARE_OUTPUT_MODE_A */
/* #define COMPARE_OUTPUT_MODE_B */

#ifdef NORMAL_MODE
#undef COMPARE_MODE
#else
#define COMPARE_MODE
#endif

#ifndef COMPARE_MODE
#undef COMPARE_MODE_A
#undef COMPARE_MODE_B
#endif

#ifdef COMPARE_OUTPUT_MODE_A
#define COM1A (0b00)
#endif
#ifdef COMPARE_OUTPUT_MODE_B
#define COM1B (0b00)
#endif

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

typedef enum
{
	NO_CLOCK_SOURCE, F_CPU_DIVIDE_1, F_CPU_DIVIDE_8, F_CPU_DIVIDE_64, F_CPU_DIVIDE_256, F_CPU_DIVIDE_1024, EXTERNAL_ON_FALLING_EDGE, EXTERNAL_ON_RISING_EDGE
}Timer1_Prescaler;

typedef enum
{
	NORMAL, PWM_PHASE_CORRECT_8_BITS, PWM_PHASE_CORRECT_9_BITS, PWM_PHASE_CORRECT_10_BITS, CTC_OCR1A, FAST_PWM_8_BITS, FAST_PWM_9_BITS, FAST_PWM_10_BITS, PWM_PHASE_AND_FREQUENCY_CORRECT_ICR1, PWM_PHASE_AND_FREQUENCY_CORRECT_OCR1A, PWM_PHASE_CORRECT_ICR1, PWM_PHASE_CORRECT_OCR1A, CTC_ICR1, FAST_PWM_ICR1 = 14, FAST_PWM_OCR1A
}Timer1_Mode;


typedef struct
{
	uint16 initial_value;
	uint16 compare_value;
	Timer1_Prescaler prescaler;
	Timer1_Mode mode;
}Timer1_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 *
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr);

/*
 * Description : The function is responsible for clear all Timer1/ICU registers,
 * disable all its interrupts and reset the global pointer value to NULL.
 *
 */
void Timer1_deInit(void);

/*
 * Description : The function is responsible for saving the address
 * of the call-back function in a global variable (pointer to function).
 *
 */
void Timer1_setCallBack(void(*a_ptr)(void));


#endif /* TIMER1_H_ */
