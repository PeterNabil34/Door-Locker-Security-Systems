 /******************************************************************************
 *
 * File Name: MC1.c
 *
 * Description: HMI_ECU is just responsible interaction with the user
 * just take inputs through keypad and display messages on the LCD.
 *
 * Created on: October 31, 2023
 *
 * Author: Peter Nabil
 *
 *******************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "timer1.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define MC1_READY                      0x10
#define MC2_READY                      0x20
#define OPEN_DOOR                      0x30
#define CHANGE_PASSWORD                0x40
#define WRONG_PASSWORD                 0x50
#define SAME                           1
#define NOT_SAME                       0
#define MATCHED                        1
#define NOT_MATCHED                    0
#define PASSWORD_SIZE                  5
#define MAX_TRIALS                     3
#define LCD_FINISHED_OPEN_DOOR         6
#define LCD_FINISHED_WRONG_PASSWORD    9

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * The function responsible for creating a new password for the system.
 */
void createPassword(uint8 *password_1, uint8 *password_2);

/*
 * Description :
 * The function responsible for sending the two passwords to the Control_ECU through the UART
 * to check if they are same or not then responds through the UART.
 */
uint8 checkSamePasswordsInControlECU(uint8 *password_1, uint8 *password_2);

/*
 * Description :
 * The function responsible for taking the password from the user as an input.
 */
void userWritePassword(uint8 *doorPassword);

/*
 * Description :
 * The function responsible for sending the password to the Control_ECU through the UART to check
 * if it matches the one saved in the EEPROM or not then responds through the UART.
 */
uint8 checkPasswordInControlECU(uint8 *password);

/*
 * Description :
 * The function responsible for starting Timer1 based on the configurations passed as parameters
 * and pass the address of the call-back function.
 */
void startTimer1(uint16 TCNT_value, uint16 OCR_value, Timer1_Prescaler prescaler, Timer1_Mode mode, void(*callBackFunctionAddress)(void));

/*
 * Description :
 * This is a function responsible for displaying "Door is Unlocking" for 15-seconds then clear
 * the LCD for 3-seconds then "Door is Locking" for 15-seconds on the LCD.
 */
void APP_openDoor(void);

/*
 * Description :
 * This is a function responsible for displaying "ERROR" on the LCD for 1-minute.
 */
void APP_wrongPassword(void);

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

uint8 g_ticks_LCD;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

int main(void)
{
	uint8 key;
	uint8 firstPassword[5];
	uint8 secondPassword[5];
	uint8 doorPassword[5];
	uint8 wrongPasswordCounter;

	SREG |= (1<<7);

	UART_ConfigType UART_Configurations = {EIGHT_BIT_DATA_MODE, DISABLED, ONE_STOP_BIT, 9600};
	UART_init(&UART_Configurations);

	LCD_init();

	do
	{
		createPassword(firstPassword, secondPassword);
	}while(NOT_SAME == checkSamePasswordsInControlECU(firstPassword, secondPassword));

	while(1)
	{
		/* Display the menu */
		LCD_clearScreen();
		LCD_displayStringRowColumn(0, 0, "+ : Open Door");
		LCD_displayStringRowColumn(1, 0, "- : Change Pass");

		do
		{
			key = KEYPAD_getPressedKey();
		}while((key != '+') && (key != '-'));

		if(key == '+')
		{
			wrongPasswordCounter = 0;
			while(wrongPasswordCounter < MAX_TRIALS)
			{
				userWritePassword(doorPassword);
				if(MATCHED == checkPasswordInControlECU(doorPassword))
				{
					g_ticks_LCD = 0;
					while(UART_recieveByte() != MC2_READY);
					UART_sendByte(OPEN_DOOR);
					APP_openDoor();
					while(g_ticks_LCD != LCD_FINISHED_OPEN_DOOR);
					break;
				}
				else
				{
					wrongPasswordCounter++;
				}
			}

			if(wrongPasswordCounter == MAX_TRIALS) /* NOT_MATCHED for 3 times */
			{
				while(UART_recieveByte() != MC2_READY);
				UART_sendByte(WRONG_PASSWORD);

				g_ticks_LCD = 0;
				APP_wrongPassword();
				while(g_ticks_LCD != LCD_FINISHED_WRONG_PASSWORD);
			}
		}
		else if(key == '-')
		{
			wrongPasswordCounter = 0;
			while(wrongPasswordCounter < MAX_TRIALS)
			{
				userWritePassword(doorPassword);
				if(MATCHED == checkPasswordInControlECU(doorPassword))
				{
					while(UART_recieveByte() != MC2_READY);
					UART_sendByte(CHANGE_PASSWORD);

					do
					{
						createPassword(firstPassword, secondPassword);
					}while(NOT_SAME == checkSamePasswordsInControlECU(firstPassword, secondPassword));
					break;
				}
				else
				{
					wrongPasswordCounter++;
				}
			}

			if(wrongPasswordCounter == MAX_TRIALS) /* NOT_MATCHED for 3 times */
			{
				while(UART_recieveByte() != MC2_READY);
				UART_sendByte(WRONG_PASSWORD);

				g_ticks_LCD = 0;
				APP_wrongPassword();
				while(g_ticks_LCD != LCD_FINISHED_WRONG_PASSWORD);
			}
		}
	}
	return 0;
}

/*
 * Description :
 * The function responsible for creating a new password for the system.
 */
void createPassword(uint8 *password_1, uint8 *password_2)
{
	uint8 i, key;
	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "plz enter pass:");
	LCD_moveCursor(1,0);

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		do
		{
			key = KEYPAD_getPressedKey();
		}while(!((key >= 0) && (key <= 9)));

		password_1[i] = key;
		//_delay_ms(600);
		LCD_displayCharacter('*');
	}

	while(13 != KEYPAD_getPressedKey());
	//_delay_ms(600);

	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "plz re-enter the");
	LCD_displayStringRowColumn(1, 0, "same pass: ");

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		do
		{
			key = KEYPAD_getPressedKey();
		}while(!((key >= 0) && (key <= 9)));

		password_2[i] = key;
		//_delay_ms(600);
		LCD_displayCharacter('*');
	}

	while(13 != KEYPAD_getPressedKey());
	//_delay_ms(600);
	LCD_clearScreen();
	LCD_moveCursor(0,0);
}

/*
 * Description :
 * The function responsible for sending the two passwords to the Control_ECU through the UART
 * to check if they are same or not then responds through the UART.
 */
uint8 checkSamePasswordsInControlECU(uint8 *password_1, uint8 *password_2)
{
	uint8 i;
	uint8 sameFlag;

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		while(UART_recieveByte() != MC2_READY);
		UART_sendByte(password_1[i]);
	}

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		while(UART_recieveByte() != MC2_READY);
		UART_sendByte(password_2[i]);
	}
	UART_sendByte(MC1_READY);
	sameFlag = UART_recieveByte();
	return sameFlag;
}

/*
 * Description :
 * The function responsible for taking the password from the user as an input.
 */
void userWritePassword(uint8 *doorPassword)
{
	uint8 i, key;
	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, "plz enter pass:");
	LCD_moveCursor(1,0);

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		do
		{
			key = KEYPAD_getPressedKey();
		}while(!((key >= 0) && (key <= 9)));

		doorPassword[i] = key;
		//_delay_ms(600);
		LCD_displayCharacter('*');
	}
	while(13 != KEYPAD_getPressedKey());
	//_delay_ms(600);
	LCD_clearScreen();
	LCD_moveCursor(0,0);
}

/*
 * Description :
 * The function responsible for sending the password to the Control_ECU through the UART to check
 * if it matches the one saved in the EEPROM or not then responds through the UART.
 */
uint8 checkPasswordInControlECU(uint8 *password)
{
	uint8 i;
	uint8 matchedFlag;

	for(i = 0; i < PASSWORD_SIZE; i++)
	{
		while(UART_recieveByte() != MC2_READY);
		UART_sendByte(password[i]);
	}

	UART_sendByte(MC1_READY);
	matchedFlag = UART_recieveByte();
	return matchedFlag;
}

/*
 * Description :
 * The function responsible for starting Timer1 based on the configurations passed as parameters
 * and pass the address of the call-back function.
 */
void startTimer1(uint16 TCNT_value, uint16 OCR_value, Timer1_Prescaler prescaler, Timer1_Mode mode, void(*callBackFunctionAddress)(void))
{
	Timer1_ConfigType Timer1_Configurations;

	Timer1_Configurations.initial_value = TCNT_value;
	Timer1_Configurations.compare_value = OCR_value; // 62500 = 8 seconds //54687 == 7 seconds
	Timer1_Configurations.prescaler = prescaler;
	Timer1_Configurations.mode = mode;

	Timer1_setCallBack(callBackFunctionAddress);
	Timer1_init(&Timer1_Configurations);
}

/*
 * Description :
 * This is a function responsible for displaying "Door is Unlocking" for 15-seconds then clear
 * the LCD for 3-seconds then "Door is Locking" for 15-seconds on the LCD.
 */
void APP_openDoor(void)
{
	g_ticks_LCD++;

	if(g_ticks_LCD == 1)
	{
		LCD_clearScreen();
		LCD_displayStringRowColumn(0, 4, "Door is");
		LCD_displayStringRowColumn(1, 3, "Unlocking");
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_openDoor);  /* 62500 = 8 seconds */
	}
	else if(g_ticks_LCD == 2)
	{
		Timer1_deInit();
		startTimer1(0x00, 54687, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_openDoor);  /* 54687 = 7 seconds */
	}
	else if(g_ticks_LCD == 3)
	{
		LCD_clearScreen();
		Timer1_deInit();
		startTimer1(0x00, 23437, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_openDoor);  /* 23437 == 3 seconds */
	}
	else if(g_ticks_LCD == 4)
	{
		LCD_displayString("Door is Locking");
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_openDoor);  /* 62500 = 8 seconds */
	}
	else if(g_ticks_LCD == 5)
	{
		Timer1_deInit();
		startTimer1(0x00, 54687, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_openDoor);  /* 54687 = 7 seconds */
	}
}

/*
 * Description :
 * This is a function responsible for displaying "ERROR" on the LCD for 1-minute.
 */
void APP_wrongPassword(void)
{
	g_ticks_LCD++;

	if(g_ticks_LCD == 1)
	{
		LCD_clearScreen();
		LCD_displayStringRowColumn(0, 5, "ERROR");
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 2)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 3)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 4)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 5)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 6)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 7)
	{
		Timer1_deInit();
		startTimer1(0x00, 62500, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 62500 = 8 seconds
	}
	else if(g_ticks_LCD == 8)
	{
		Timer1_deInit();
		startTimer1(0x00, 3125, F_CPU_DIVIDE_1024, CTC_OCR1A, APP_wrongPassword);  // 3125 = 4 seconds
	}
}


