#include <stm32f0xx.h>
#include "commands.h"
#include "main.h"
#include "usart.h"
#include "param_flash.h"
#include "main1.h"
#include "timer.h"

extern uint8_t lock ;
uint8_t send_buf[8];
extern uint8_t flag_1;
extern uint16_t adr;
void sent_buffer ()
{
}

void close_lock (void)
{
	//Reverse_On();
	Direction_down();
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	rele_On();
	lock = 1;
}

void open_lock (void)
{
	Direction_up();
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	rele_On();
	lock = 0;
}

void commands (uint8_t *b)
{	
	uint16_t param;
	if ( (b [8] << 8 | (b[9])) == adr ) {		//#define adr 0x3451
		set_flag_1 (1);			// готовность ответа
		send_buf[0] = b[0] | 128;
		
		
		if (b[0] == 1){			// команда инвертировать замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			if (lock){			// замок в закрытом состоянии 
				open_lock ();
			}
			else {
				close_lock ();
			}
			
			if (b[1] != 0){
				set_t(b[1]);
			}
			else {
				set_t(Read16ParamFlash(1) & 255);
			}
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 3){	// команда открыть замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			open_lock ();
			
			if (b[1] != 0){
				set_t(b[1]);
			}
			else {
				set_t(Read16ParamFlash(1) & 255);
			}
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 2){	// команда закрыть замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			close_lock ();
			
			if (b[1] != 0){
				set_t(b[1]);
			}
			else {
				set_t(Read16ParamFlash(1) & 255);
			}
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 4){	// запрос статуса открыть замок
			send_buf[1] = (GPIOA->IDR & 2) >> 1;	// чтение датчика двери
		}
		else if (b[0] == 5){	// поменять параметр, номер которого указанн в b[1]
			send_buf[4] = WriteParamFlash(b[1], (b[3] << 8) | b[2]);
			param = Read16ParamFlash(b[1]);
			send_buf[1] = b[1];
			send_buf[3] = param >> 8;
			send_buf[2] = param & 0xff;
		}
		else if (b[0] == 6){	// прочитать параметр, номер которого указанн в b[1]
			param = Read16ParamFlash(b[1]);
			send_buf[3] = param >> 8;
			send_buf[2] = param & 0xff;
			//t_val = Read16ParamFlash(1) & 255;
		}
		else {
		}
	}
	b[8] = 0;
	b[9] = 0;
	
}
