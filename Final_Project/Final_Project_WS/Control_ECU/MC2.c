 /******************************************************************************
 *
 * File Name: MC2.c
 *
 * Description: CONTROL_ECU is responsible for all the processing
 * and decisions in the system like password checking, open the door
 * and activate the system alarm.
 *
 * Created on: October 31, 2023
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "twi.h"
#include "external_eeprom.h"
#include "timer1.h"
#include "DC_Motor.h"
#include "buzzer.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define MC1_READY           0x10
#define MC2_READY           0x20
#define OPEN_DOOR           0x30
#define CHANGE_PASSWORD     0x40
#define WRONG_PASSWORD      0x50
#define PASSWORD_ADDRESS    0x0310
#define SAME                1
#define NOT_SAME            0
#define MATCHED             1
#define NOT_MATCHED         0
#define PASSWORD_SIZE       5
#define MAX_TRIALS          3
#define DC_MOTOR_FINISHED   7
#define BUZZER_FINISHED     10

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * The function responsible for receiving two passwords from the HMI_ECU through the UART.
 */
void receiveTwoPasswordsFrom_HMI_ECU(uint8 *password_1, uint8 *password_2);

/*
 * Description :
 * The function responsible for check if the two passwords are the same or not then respond to the HMI_ECU though the UART.
 */
uint8 checkSamePasswords(uint8 *password_1, uint8 *password_2);

/*
 * Description :
 * The function responsible for saving the password in the EEPROM based on the address passed as a parameter.
 */
void savePasswordInEEPROM(uint16 EEPROM_location, uint8 *password);

/*
 * Description :
 * The function responsible for receiving the password from the HMI_ECU through the UART.
 */
void receivePasswordFrom_HMI_ECU(uint8 *password);

/*
 * Description :
 * The function responsible for checking if the password matches the one saved in the EEPROM or not
 * then responds to the HMI_ECU through the UART.
 */
uint8 checkOnPassword(uint16 EEPROM_location, uint8 *HMI_password);

/*
 * Description :
 * The function responsible for starting Timer1 based on the configurations passed as parameters
 * and pass the address of the call-back function.
 */
void startTimer1(uint16 TCNT_value, uint16 OCR_value, Timer1_Prescaler prescaler, Timer1_Mode mode, void(*callBackFunctionAddress)(void));

/*
 * Description :
 * The function responsible for rotating the DC-Motor clockwise for 15-seconds
 * then holding it for 3-seconds then rotating the DC-Motor anti-clockwise.
 */
void APP_DcMotor(void);

/*
 * Description :
 * The function responsible for activation the buzzer for 1-minute.
 */
void APP_buzzer(void);

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

uint8 g_ticks_DCMotor;
uint8 g_ticks_buzzer;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

int main(void)
{
	uint8 firstPassword[5];
	uint8 secondPassword[5];
	uint8 doorPassword[5];
	uint8 samePasswordFlag;
	uint8 wrongPasswordCounter;
	uint8 option;

	SREG |= (1<<7);

	TWI_ConfigType TWI_Configurations = {0b0000001, 0x02, ONE};
	TWI_init(&TWI_Configurations);

	UART_ConfigType UART_Configurations = {EIGHT_BIT_DATA_MODE, DISABLED, ONE_STOP_BIT, 9600};
	UART_init(&UART_Configurations);

	DcMotor_Init();

	Buzzer_init();

	samePasswordFlag = SAME;
	do
	{
		if(samePasswordFlag != SAME)
		{
			while(UART_recieveByte() != MC1_READY);
			UART_sendByte(NOT_SAME);
		}
		receiveTwoPasswordsFrom_HMI_ECU(firstPassword, secondPassword);
		samePasswordFlag++;
	}while(NOT_SAME == checkSamePasswords(firstPassword, secondPassword));

	while(UART_recieveByte() != MC1_READY);
	UART_sendByte(SAME);

	savePasswordInEEPROM(PASSWORD_ADDRESS, firstPassword);

	while(1)
	{
		wrongPasswordCounter = 0;
		while(wrongPasswordCounter < MAX_TRIALS)
		{
			receivePasswordFrom_HMI_ECU(doorPassword);
			if(MATCHED == checkOnPassword(PASSWORD_ADDRESS, doorPassword))
			{
				while(UART_recieveByte() != MC1_READY);
				UART_sendByte(MATCHED);
				break;
			}
			else /* NOT_MATCHED */
			{
				while(UART_recieveByte() != MC1_READY);
				UART_sendByte(NOT_MATCHED);
				wrongPasswordCounter++;
			}
		}

		if(wrongPasswordCounter == MAX_TRIALS)
		{
			UART_sendByte(MC2_READY);
			option = UART_recieveByte();

			if(WRONG_PASSWORD == option)
			{
				g_ticks_buzzer = 0;
				APP_buzzer();
				while(g_ticks_buzzer != BUZZER_FINISHED);
			}
		}
		else
		{
			UART_sendByte(MC2_READY);
			option = UART_recieveByte();

			if(OPEN_DOOR == option)
			{
				g_ticks_DCMotor = 0;
				APP_DcMotor();
				while(g_ticks_DCMotor != DC_MOTOR_FINISHED);
			}
			else if(CHANGE_PASSWORD == option)
			{
				samePasswordFlag = SAME;
				do
				{
					if(samePasswordFlag != SAME)
					{
						while(UART_recieveByte() != MC1_READY);
						UART_sendByte(NOT_SAME);
					}
					receiveTwoPasswordsFrom_HMI_ECU(firstPassword, secondPassword);
					samePasswordFlag++;
				}while(NOT_SAME == checkSamePasswords(firstPassword, secondPassword));

				while(UART_recieveByte() != MC1_READY);
				UART_sendByte(SAME);

				savePasswordInEEPROM(PASSWORD_ADDRESS, firstPassword);
			}
		}
	}
	return 0;
}

/*
 * Description :
 * The function responsible for receiving two passwords from the HMI_ECU through the UART.
 */
void receiveTwoPasswordsFrom_HMI_ECU(uint8 *password_1, uint8 *password_2)
{
	uint8 i;

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		UART_sendByte(MC2_READY);
		password_1[i] = UART_recieveByte();
	}

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		UART_sendByte(MC2_READY);
		password_2[i] = UART_recieveByte();
	}
}

/*
 * Description :
 * The function responsible for check if the two passwords are the same or not then respond to the HMI_ECU though the UART.
 */
uint8 checkSamePasswords(uint8 *password_1, uint8 *password_2)
{
	uint8 i;
	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		if(password_1[i] != password_2[i])
		{
			return NOT_SAME;
		}
	}
	return SAME;
}

/*
 * Description :
 * The function responsible for saving the password in the EEPROM based on the address passed as a parameter.
 */
void savePasswordInEEPROM(uint16 EEPROM_location, uint8 *password)
{
	uint8 i;
	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		EEPROM_writeByte(EEPROM_location, password[i]);
		EEPROM_location += 1;
		_delay_ms(10);
	}
}

/*
 * Description :
 * The function responsible for receiving the password from the HMI_ECU through the UART.
 */
void receivePasswordFrom_HMI_ECU(uint8 *password)
{
	uint8 i;
	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		UART_sendByte(MC2_READY);
		password[i] = UART_recieveByte();
	}
}

/*
 * Description :
 * The function responsible for checking if the password matches the one saved in the EEPROM or not
 * then responds to the HMI_ECU through the UART.
 */
uint8 checkOnPassword(uint16 EEPROM_location, uint8 *HMI_password)
{
	uint8 i;
	uint8 savedPassword[5];

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		EEPROM_readByte(EEPROM_location, savedPassword+i);
		EEPROM_location += 1;
	}

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		if(HMI_password[i] != savedPassword[i])
		{
			return NOT_MATCHED;
		}
	}
	return MATCHED;
}

/*
 * Description :
 * The function responsible for starting Timer1 based on the configurations passed as parameters
 * and pass the address of the call-back function.
 */
void startTimer1(uint16 TCNT_value, uint16 OCR_value, Timer1_Prescaler prescaler, Timer1_Mode mode, void(*callBackFunctionAddress)(void))
{
	Timer1_ConfigType Timer1_Configuration;

	Timer1_Configuration.initial_value = TCNT_value;
	Timer1_Configuration.compare_value = OCR_value; // 62500 = 8 seconds //54687 == 7 seconds
	Timer1_Configuration.prescaler = prescaler;
	Timer1_Configuration.mode = mode;

	Timer1_setCallBack(callBackFunctionAddress);
	Timer1_init(&Timer1_Configuration);
}

/*
 * Description :
 * The function responsible for rotating the DC-Motor clockwise for 15-seconds
 * then holding it for 3-seconds then rotating the DC-Motor anti-clockwise.
 */
void APP_DcMotor(void)
{
	g_ticks_DCMotor++;

	if(g_ticks_DCMotor == 1)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_DcMotor);  // 62500 = 8 seconds
		DcMotor_Rotate(CLOCKWISE, 100);
	}
	else if(g_ticks_DCMotor == 2)
	{
		Timer1_deInit();
		startTimer1(0x00, 54687, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_DcMotor);  //54687 == 7 seconds
	}
	else if(g_ticks_DCMotor == 3)
	{
		Timer1_deInit();
		startTimer1(0x00, 23437, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_DcMotor);  //23437 == 3 seconds
		DcMotor_Rotate(STOP, 100);
	}
	else if(g_ticks_DCMotor == 4)
	{
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_DcMotor);  // 62500 = 8 seconds
		DcMotor_Rotate(ANTI_CLOCKWISE, 100);
	}
	else if(g_ticks_DCMotor == 5)
	{
		Timer1_deInit();
		startTimer1(0x00, 54687, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_DcMotor);  //54687 == 7 seconds
	}
	else if(g_ticks_DCMotor == 6)
	{
		DcMotor_Rotate(STOP, 100);
		Timer1_deInit();
	}
}

/*
 * Description :
 * The function responsible for activation the buzzer for 1-minute.
 */
void APP_buzzer(void)
{
	g_ticks_buzzer++;

	if(g_ticks_buzzer == 1)
	{
		Buzzer_on();
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 2)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 3)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 4)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 5)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 6)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 7)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 62500 = 8 seconds
	}
	else if(g_ticks_buzzer == 8)
	{
		Timer1_deInit();
		startTimer1(0x00, 3125, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_buzzer);  // 3125 = 4 seconds
	}
	else if(g_ticks_buzzer == 9)
	{
		Buzzer_off();
	}
}

