#include "timer.h"
#include "usart.h"
#include "main.h"
#include "main1.h"
#include "param_flash.h"
#include "modbus.h" 

uint16_t t;
extern uint8_t lock ;

extern UART_DATA uart1;//структуры для усарта
uint16_t *stat1 = &uart1.reg[10]; 

void set_t (uint16_t val)
{
	t = val;
}

void init_TIM2 (void)
{
}

void init_TIM3 (void)
{
    /*Инициализация таймера TIM3, без его включения */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;	//Тактирование таймера TIM3
	TIM3->PSC	=4800; 
	TIM3->ARR 	= 1000;					//Период выходного сигнала T = 0.5 c.	
	TIM3->CR1 	|= TIM_CR1_CEN;			//Старт счета таймера 
	TIM3->DIER	= TIM_DIER_UIE; 		// Enable update interrupt (timer level)
	NVIC_EnableIRQ(TIM3_IRQn); 			// Enable interrupt from TIM2 (NVIC level)
}

void TIM3_IRQHandler(void)
{
	static uint8_t s1, s2;
	
	TIM3->SR &= ~TIM_SR_UIF; //Сбрасываем флаг UIF
	if (t != 0){
		t--;
		if (t == 0){
			all_off();
			*stat1 &= ~4;
			set_flag_1(1);
		}
		else{
			if (lock & (!(GPIOA->IDR & sen_close))){
				s1++;
				if (s1 > 3) {
					all_off();
					*stat1 &= ~4; 
					t = 0;
				}
			}
			else{
				s1 = 0;
			}
			if (!lock & (!(GPIOA->IDR & sen_open))){
				s2++;
				if (s2 > 3) {
					all_off();
					*stat1 &= ~4;
					t = 0;
				}
			}
			else {
				s2 = 0;
			}
				
		}
	}
}
