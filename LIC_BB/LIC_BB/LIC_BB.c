/*
 * LIC_CV_01.c
 *
 * Created: 28.7.2015 17:01:20
 *  Author: Lukas
 */ 


//#include <avr/iom32.h>
#include <avr/io.h>
//#include <avr/iom32.h>
#include "inc/AllInit.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include "inc/common_defs.h"
#include "inc/defines.h"
#include "inc/timer.h"
#include "inc/uart_types.h"
#include "inc/uart_tri_0.h"
#include "inc/Tribus_types.h"
#include "inc/Tribus.h"
#include <stdlib.h>
// Standard Input/Output functions
#include <stdio.h>
#include <string.h>

//Nastavení pøerušení pro BlackBox
#define INT1_SHIELD		INT1_vect
#define INT2_SERVIS		INT2_vect
#define INT3_CHAMBER	INT3_vect
#define INT4_Channel	INT4_vect
#define INT5_Channel	INT5_vect

volatile unsigned char Check_SHIELD;
volatile unsigned char Check_SERVIS;
volatile unsigned char Check_CHAMBER;

#define _IL_SHIELD_ON (PIND & (1 << PIND1))
#define _IL_SERVIS_ON (PIND & (1 << PIND2))
#define _IL_CHAMBER_ON (PIND & (1 << PIND3))

//PD1 - Shield
//PD2 - Servis
//PD3 - Chamber

// User variables
word timeout_A_max = 500;
word timeout_B_max = 500;
byte mask_A = 0x03;
byte mask_B = 0x0C;

byte armed_A = false;
byte armed_B = false;
word timeout_A = 0;
word timeout_B = 0;
byte mask_M = 0;


//Pøerušení pro interlocky
ISR(INT1_SHIELD);
ISR(INT2_SERVIS);
ISR(INT3_CHAMBER);
ISR(INT4_Channel);
ISR(INT5_Channel);
void PreruseniBlackBox_Init(void);

//Samotné pøerušení
ISR(INT1_SHIELD)
{
	//Zde se musí dát nastavení do log.0 pro výstup PA7 Laser IN
	if (!_IL_SERVIS_ON)
	{
		PORTA &= ~(1 << PA7);
		Check_SHIELD = TRUE;
	} 
}

ISR(INT2_SERVIS)
{
	Check_SERVIS = TRUE;
}

ISR(INT3_CHAMBER)
{
//Zde se musí dát nastavení do log.0 pro výstup PA7 Laser IN
	if (!_IL_SERVIS_ON)
	{
		PORTA &= ~(1 << PA7);
		Check_SHIELD = TRUE;
	}
}

ISR(INT4_Channel)
{
	if(armed_A)
	{
		timeout_A = timeout_A_max;
		PORTC &= ~(mask_A);
		armed_A = FALSE;
	}
}

ISR(INT5_Channel)
{
	if(armed_B)
	{
		timeout_B = timeout_B_max;
		PORTC &= ~(mask_B);
		armed_B = FALSE;
	}
}

/******************************************************/
ISR(TIMER1_COMPA_vect)
{
	if (timeout_A > 0) {
		if (--timeout_A == 0) {
			PORTC |= mask_A;
		};
	}
	if (timeout_B > 0) {
		if (--timeout_B == 0) {
			PORTC |= mask_B;
		};
	}
}


// Init pro Tribus
volatile byte timer0_flag = 0; // T = 10ms
byte led_timer = 0;



void send_data(void)
{
	uart0_put_data((byte *) &TB_bufOut);
}

//----------------------------------------------------------
ISR(TIMER1_CAPT_vect) {
	// T = 10ms
	timer0_flag = true;
}

//----------------------------------------------------------
void process_timer_100Hz(void)
{
	if (timer0_flag) { // T = 10ms
		timer0_flag = false;
		uart0_ISR_timer();
		if (led_timer > 0) {
			led_timer--;
			if (led_timer == 0) {
				PORTB ^= (1 << PB4);
			}
		}
	}
}

void try_receive_data(void)
{
	byte i;
	byte *ptr;
	
	if (uart0_flags.data_received)
	{
		ptr = uart0_get_data_begin();
		for (i=0; i<9; i++)
		{
			TB_bufIn[i] = *ptr;
			ptr++;
		}
		uart0_get_data_end();
		uart0_flags.data_received = FALSE;
		if (TB_Read() == 0)
		{
			switch (TB_Decode())
			{
				case TB_CMD_BLACKBOX:
					switch (TB_bufIn[TB_BUF_TYPE])
					{
						case BB_ACTIVE:
							switch (TB_bufIn[TB_BUF_MOTOR])
							{
								//Nastavení pro pøerušení ISR(INT4_Channel);
								case 0:
									if (TB_Value == 1)
									{
										armed_A = TRUE;	
										TB_SendAckOK();
									}
									else
									{
										TB_SendAck(TB_ERR_NOK, TB_Value);
									}
									break;
								//Nastavení pro pøerušení ISR(INT5_Channel)
								case 1:
									if (TB_Value == 1)
									{
										armed_B = TRUE;
										TB_SendAckOK();
									}
									else
									{
										TB_SendAck(TB_ERR_NOK, TB_Value);
									}
									break;
								default:
									TB_SendAck(TB_ERR_NOK,0);
									break;
							}
							break;
						case BB_SETOUTPUT:
							//Potvrzovací pøíkaz pokud je motor 0 tak se vezme a nastaví na výstup
							if (TB_bufIn[TB_BUF_MOTOR] == 0)
							{
								PORTC |=  (TB_bufIn[4] & 0xF0);
								TB_SendAckOK();
							}
							//Potvrzovací pøíkaz pokud je motor 1 tak se odpojí výstupy
							else if (TB_bufIn[TB_BUF_MOTOR] == 1)
							{
								PORTC &= ~(TB_bufIn[4] & 0xF0);
								TB_SendAckOK();
							}
							break;

						case BB_SETTIME:
							//Nastavení Timer A v hodnotì value je hodnota timeru
							if (TB_bufIn[TB_BUF_MOTOR] == 0)
							{
								timeout_A_max = (TB_Value << 8) & 0xFFFF;
								TB_SendAck(TB_ERR_OK, TB_Value);
							}
							//Nastavení Timer B v hodnotì value je hodnota timeru
							else if (TB_bufIn[TB_BUF_MOTOR] == 1)
							{
								timeout_B_max = (TB_Value << 8) & 0xFFFF;
								TB_SendAck(TB_ERR_OK, TB_Value);
							}
							else
							{
								TB_SendAck(TB_ERR_NOK, 0);
							}
							break;
						case BB_SETMASK:
							//Nastavení Timer A v hodnotì value je hodnota timeru
							if (TB_bufIn[TB_BUF_MOTOR] == 0)
							{
								mask_A = (TB_Value << 24) & 0x0F;
								
								TB_SendAck(TB_ERR_OK, TB_Value);
							}
							//Nastavení Timer B v hodnotì value je hodnota timeru
							else if (TB_bufIn[TB_BUF_MOTOR] == 1)
							{
								mask_B = (TB_Value << 24) & 0x0F;
								TB_SendAck(TB_ERR_OK, TB_Value);
							}
							else
							{
								TB_SendAck(TB_ERR_NOK, 0);
							}
							break;
					}
					break;
					
					
				case TB_CMD_INTERLOCK:
					switch (TB_bufIn[TB_BUF_TYPE])
					{
						//info
						case IL_INFO:
							if (TB_bufIn[TB_BUF_MOTOR] == 0)
							{
								TB_SendAck(TB_ERR_OK, _IL_SERVIS_ON);	
							}
							else if (TB_bufIn[TB_BUF_MOTOR] == 1)
							{
								TB_SendAck(TB_ERR_OK, _IL_CHAMBER_ON + _IL_SHIELD_ON);	
							}
							else
							{
								TB_SendAck(TB_ERR_NOK, 0);
							}
						// Shield
						case IL_LASER:
							// Pokud je Motor nastaven do 1 neboli true tak chce povolit laser
							if (TB_bufIn[TB_BUF_MOTOR] == 0)
							{
								//Je v servisnim modu?
								if (_IL_SHIELD_ON || _IL_CHAMBER_ON)
								{
									if (_IL_SERVIS_ON)
									{
										if (TB_Value == 1)
										{
											//Vypne Interlock
											PORTA |= (1 << PA7);
											TB_SendAck(TB_ERR_OK, 1);
										}
										else if (TB_Value == 0)
										{
											//Zapne interlock
											PORTA &= ~(1 << PA7);
											TB_SendAck(TB_IL_ERR, 1);
										}
									}
								}
								else
								{
									if (TB_Value == 1)
									{
										//Vypne Interlock
										PORTA |= (1 << PA7);
										TB_SendAck(TB_ERR_OK, 1);
									} 
									else if (TB_Value == 0)
									{
										//Zapne interlock
										PORTA &= ~(1 << PA7);
										TB_SendAck(TB_IL_ERR, 1);
									}
									else
									{
										TB_SendAck(TB_ERR_NOK, 0);
									}
								} 
							}
							//Nastavení Timer B v hodnotì value je hodnota timeru
							else
							{
								
							}
							break;
							
						default:
							TB_SendAck(TB_ERR_NOK, 0);
							break;
					} 
					break;
			}
		}
	}
}

void PreruseniBlackBox_Init(void)
{
	//Nastavení vstupních externích pøerušení
	DDRD |= ~((1 << PD1) | (1 << PD2) | (1 << PD3));
	
	//Nastavení Channel 1-4 pro nastavování a IN_Laser
	DDRA |= (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3) | (1 << PA7);
	//Nastavení Pull-up resistorù
	PORTA = 0xFF;
	
	// Nastavení INT1 na nástupnou hranu
	EICRA |= (1 << ISC10) | (1 << ISC11);
	// Nastavení INT2 na nástupnou hranu
	EICRA |= (1 << ISC20) | (1 << ISC21);
	// Nastavení INT3 na nástupnou hranu
	EICRA |= (1 << ISC30) | (1 << ISC31);

	//Nastavení Timer1 = CTC
	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12) | (1 << CS10);
	OCR1A = 1474; // 14,7456MHz / 1474 ~ 10kHz

	//Povolení pøerušení pro INT 1 - 5
	EIMSK |= (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
}


int main(void)
{
	//Nastavení Systemového enable pro RS485 pro UART0	
	DDRD |= (1 << PD0);

	PreruseniBlackBox_Init();
		
	timer_init();
	uart0_init();
	TB_Callback_setBaud = &uart0_set_baud;
	TB_Callback_TX = &send_data;
	TB_Init((void*) 0x10); // addr in eeprom with settings
	
	sei();
	
    while(1)
    {
		process_timer_100Hz();
		uart0_process();
		try_receive_data();
    }
}
