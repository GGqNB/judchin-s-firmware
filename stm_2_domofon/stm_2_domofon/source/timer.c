#include "timer.h"
#include "usart.h"
#include "main.h"
#include "main1.h"
#include "param_flash.h"

uint16_t t;

void set_t (uint16_t val)
{
	t = val;
}

void init_TIM2 (void)
{
    /*Инициализация таймера TIM2 для формирования сигнала ШИМ используется канал 2 (TIM2_CH2)*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;	//Тактирование таймера TIM2
	
	TIM2->PSC=0; 
	TIM2->ARR = 23999;					//Период выходного сигнала T = 0,5mS
	
	//TIM2->CCR4 = 10000;						//Длительность импульса. В данном случае Duty cycle = 0%
		
	TIM2->CCMR2 |= TIM_CCMR2_OC4PE;		//Включен режим предварительной загрузки регистра сравнения
	TIM2->CCMR2 |= (TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1);//OC2M = 110 - PWM mode 1
	TIM2->CCER 	|= TIM_CCER_CC4E;		//Выход канала захвата/сравнения включен
	TIM2->CCER 	|= TIM_CCER_CC4P;		//Полярность выходного сигнала
	TIM2->CR1  	|= TIM_CR1_CEN;			//Старт счета таймера
	TIM2->CCR4 = 20000;
	
	//TIM2->DIER = TIM_DIER_UIE; // Enable update interrupt (timer level)
	//NVIC_EnableIRQ(TIM2_IRQn); // Enable interrupt from TIM2 (NVIC level)
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
	TIM3->SR &= ~TIM_SR_UIF; //Сбрасываем флаг UIF
	if (t != 0){
		t--;
		if (t == 0){
			all_off();
			set_flag_1(1);
			NVIC_EnableIRQ(EXTI4_15_IRQn);   
		}
	}
}
