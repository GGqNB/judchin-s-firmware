#include <delay.h>
#include "main.h"
#define timeout  10

#define key1_tim_val 10000
#define key2_tim_val 10000

char RX_timeout = 0;	//

extern __IO uint32_t timeout_card1;
extern __IO uint32_t timeout_card2;

uint16_t key1_tim, key2_tim;

// ***** from main.c *****
void SysTick_conf (uint32_t TimerTick) // настраиваем таймер, но не разрешаем работу
{
	SysTick->LOAD=TimerTick;		// Загрузка значения
	SysTick->VAL=TimerTick;			// Обнуляем таймеры и флаги. Записью, помните? 
	SysTick->CTRL=	SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;  
}
//Функция временной задержки
void Delay_ms(__IO uint32_t nTime)
{
	//SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
	// разрешение работы таймера
    //SysTick->CTRL =	SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	TimingDelay = nTime;
    while(TimingDelay != 0);
	//SysTick->CTRL = 0;
}

//Обработчик прерывания системного таймера
void SysTick_Handler(void)
{
	if (TimingDelay != 0x00){
        TimingDelay--;
    }
	if (timeout_card1 != 0) {
		timeout_card1 --;
		if ( timeout_card1 == 0) {
			set_timeout_card();
		}
	}
	if (timeout_card2 != 0) {
		timeout_card2 --;
		if ( timeout_card2 == 0) {
			set_timeout_card();
		}
	}
	if (!(GPIOA->IDR & (1 << 7))){
		key1_tim ++;
		if (key1_tim > key1_tim_val){
			set_flag_1(2);
		}
	}
	else {
		key1_tim = 0;
	}
	
	if (!(GPIOA->IDR & (1 << 4))){
		key2_tim ++;
		if (key2_tim > key2_tim_val){
			set_flag_1(4);
		}
	}
	else {
		key2_tim = 0;
	}
}

// ***** from timer.c *****
void init_TIM14 (void) 	/*Инициализация таймера TIM14 */
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;	//Тактирование таймера TIM3
	TIM14->PSC	=8000/10; 
	TIM14->ARR 	= 100;					//Период выходного сигнала T = 0.01 c.	
	TIM14->CR1 	|= TIM_CR1_CEN;			//Старт счета таймера 
	TIM14->DIER	= TIM_DIER_UIE; 		// Enable update interrupt (timer level)
	NVIC_EnableIRQ(TIM14_IRQn); 			// Enable interrupt from TIM14 (NVIC level)
}


void TIM14_IRQHandler (void)
{
	TIM14->SR &= ~TIM_SR_UIF; //Сбрасываем флаг UIF
}
