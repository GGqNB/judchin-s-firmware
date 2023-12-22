#include <stm32f0xx.h>
#include "commands.h"
#include "main.h"
#include "usart.h"
#include "param_flash.h"
#include "timer.h"
#include "modbus.h"

extern uint8_t lock ;
uint8_t send_buf[8];
extern uint8_t flag_1;


// **** структура ModBus
extern UART_DATA uart1;//структуры для усарта

uint16_t *stat_1 = &uart1.reg[10]; 
uint16_t *adr_can = &uart1.reg[0]; 

uint16_t *t_up  = &uart1.reg[2]; 
uint16_t *t_down  = &uart1.reg[3]; 

void sent_buffer ()
{
}

void close_lock (void)
{
	if (GPIOA->IDR & (sen_close)){
		//Reverse_On();
		Direction_down();
		rele_On();
		
		//NVIC_DisableIRQ(EXTI4_15_IRQn);
		lock = 1;
		*stat_1 = 4 | 2;			// вентиль закрыт
	}
}

void open_lock (void)
{
	if (GPIOA->IDR & (sen_open)){
		Direction_up();
		rele_On();
		//NVIC_DisableIRQ(EXTI4_15_IRQn);
		lock = 0;
		*stat_1 = 4 | 1;			// вентиль открыт
	}
}

void commands_modbus (uint8_t b)
{	
	//if ( (b [8] << 8 | (b[9])) == adr ) {		//#define adr 0x3451
		//set_flag_1 (1);			// готовность ответа

		if (b == 1){			// команда открыть замок
			//clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			open_lock ();
			
			//Motor_On();
			set_t(*t_up);
			//uart1.reg[12] = 0;
			
		}
		
		else if (b == 2){			// команда закрыть замок
			//clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			close_lock ();
			//Motor_On();
			set_t(*t_down);
		}
		
		else if (b == 3){			// СТОП
			Motor1_Off();
			Motor2_Off();
			rele_Off();
			//NVIC_EnableIRQ(EXTI2_3_IRQn);  
			set_t(0);
		}
		
	//}
}

void commands (uint8_t *b)
{	
	if ( (b [8] << 8 | (b[9])) == *adr_can ) {		//#define adr 0x3451
		set_flag_1 (1);			// готовность ответа
		send_buf[0] = b[0] | 128;
			
		if (b[0] == 1){			// команда инвертировать замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			if (lock){			// замок в закрытом состоянии 
				open_lock ();
				set_t(*t_up);
			}
			else {
				close_lock ();
				set_t(*t_down);
			}
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 3){			// команда открыть замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			open_lock ();
			
			//Motor_On();
			set_t(*t_up);
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 2){			// команда закрыть замок
			clr_flag_1(1);		// ответ будет готов после окончания работы мотора
			close_lock ();
			//Motor_On();
			set_t(*t_down);
			
			send_buf[1] = lock;
		}
		
		else if (b[0] == 4){	// запрос статуса открыть замок
			send_buf[1] = (GPIOA->IDR & 2) >> 1;	// чтение датчика двери
		}
		else if (b[0] == 5){	// записать параметр во флеш
			if (b[1] < 10){
				Write16ParamFlash(b[1], (b[3]<<8 | b[2]));
			}
		}
		else if (b[0] == 6){
		}
		else {
		}
	}
	b[8] = 0;
	b[9] = 0;
	
}
