/*
 * LIC_CV_01.c
 *
 * Created: 28.7.2015 17:01:20
 *  Author: Lukas
 */ 


#include <avr/iom32.h>
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


//#include <>

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
				case TB_CMD_VZOREK:
					if ()
					{
						TB_SendAckOK();
					}
					else
					{
						TB_SendAckOK();
					}
			}
		}
	}
}

int main(void)
{
/*
	DDRA |= (1 << DDA7) | (1 << DDA6) | (1 << DDA5) | (1 << DDA4) | (1 << DDA3) | (1 << DDA2) | (1 << DDA1);
	DDRB |= (1 << DDB4) | (1 << DDB3);
	DDRC |= (1 << DDC7) | (1 << DDC6) | (1 << DDC5) | (1 << DDC4);
	DDRD |= (1 << DDD4) | (1 << DDD3) | (1 << DDD2);
*/

	
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
