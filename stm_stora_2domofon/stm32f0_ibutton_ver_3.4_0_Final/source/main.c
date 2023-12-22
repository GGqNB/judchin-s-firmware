//
// Пример проекта для платы 
// Judchin
// 

//#include "nRF24L01.h"
#include "usart.h"
#include "main.h"
#include "main1.h"
#include "timer.h"
#include <stm32f0xx.h>
#include "param_flash.h"
#include "delay.h"
#include "ext_int.h"
#include "commands.h"
#include "modbus.h"


uint8_t lock;					// состояние замка
volatile uint8_t flag;

uint8_t flag_1 = 0;

extern uint8_t send_buf[8];
extern uint16_t t;

char deb_main;


// **** структура ModBus
extern UART_DATA uart1;//структуры для усарта

uint16_t *adr_pcb = &uart1.reg[0];
uint16_t *t_up_m  = &uart1.reg[2]; 
uint16_t *t_down_m  = &uart1.reg[3]; 

uint16_t *stat_sen = &uart1.reg[9]; 
uint16_t *stat = &uart1.reg[10]; 
uint16_t *Lamp0 = &uart1.reg[11]; 
uint16_t *Lamp1 = &uart1.reg[12];
uint16_t *Lamp2 = &uart1.reg[13];
uint16_t *Port = &uart1.reg[15];
uint16_t *port_old = &uart1.reg[20];

/*******************************************************************/
void init_gpio (void)
{
	// Включить тактирование порта A
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOFEN;
    GPIOA->MODER = 	GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0 |
					GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
	GPIOA->PUPDR = GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR3_0 | GPIO_PUPDR_PUPDR14_0;
	GPIOF->MODER =  GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0;
	GPIOF->BSRR = GPIO_BSRR_BR_0;
}

void init_EXTI()
{
	
    // Настраиваем EXTI1 и EXTI2  на выводы порта А    
    SYSCFG->EXTICR[3]|= SYSCFG_EXTICR4_EXTI14_PA; //     http://eugenemcu.ru/publ/13-1-0-80
    // Разрешаем прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR14; 
	EXTI->FTSR |=EXTI_FTSR_TR14;		// Прерывание по спаду уровня на ноге 14 порта привязанного к EXTI
    NVIC_SetPriority(EXTI4_15_IRQn, 0); // функция, которая позволяет изменить приоритет прерывания 
	NVIC_EnableIRQ(EXTI4_15_IRQn);   
	
	// ********* PA.0, PA.1 ************
	SYSCFG->EXTICR[0]|= SYSCFG_EXTICR1_EXTI0_PA | SYSCFG_EXTICR1_EXTI1_PA; //     http://eugenemcu.ru/publ/13-1-0-80
    // Разрешаем прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1; 
    EXTI->RTSR 	|=EXTI_RTSR_TR0;	// Прерывание по нарастанию уровня на ноге 0 порта привязанного к EXTI
	EXTI->FTSR |=EXTI_FTSR_TR1;		// Прерывание по спаду уровня на ноге 1 порта привязанного к EXTI
    NVIC_SetPriority(EXTI0_1_IRQn, 0); // функция, которая позволяет изменить приоритет прерывания 
	NVIC_EnableIRQ(EXTI0_1_IRQn);   
	
	// ********* PA.4, PA.5 ************
	SYSCFG->EXTICR[1]|= SYSCFG_EXTICR2_EXTI4 | SYSCFG_EXTICR2_EXTI5; //     http://eugenemcu.ru/publ/13-1-0-80
    // Разрешаем прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR4 | EXTI_IMR_MR5;  
    EXTI->FTSR 	|=EXTI_FTSR_TR4 | EXTI_FTSR_TR5;	// Прерывание по спаду уровня на ноге 2, 3 порта привязанного к EXTI
    NVIC_SetPriority(EXTI4_15_IRQn, 0); // функция, которая позволяет изменить приоритет прерывания 
	NVIC_EnableIRQ(EXTI4_15_IRQn);   
}


void init (void)
{
    init_gpio();
	Init_usart();
	//init_EXTI();
	//SysTick_conf(SystemCoreClock/100000 - 1); // прерывание каждые 10 мкс
	
	init_TIM3();
	//init_TIM14();
}
void USART_to_SendData(uint16_t Data1)
{
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (Data1 & (uint16_t)0x01FF);
}
void set_flag_1 (int16_t f)
{
	flag_1 |= f;
}

void clr_flag_1 (int16_t f)
{
	flag_1 &= ~f;
}

void send_pack (uint16_t par, uint16_t a, uint32_t card_n)
{
	WR_on();
	USART_to_SendData(par >> 8);
	USART_to_SendData(par & 0xff);
	USART_to_SendData(0);
	USART_to_SendData(0);
	USART_to_SendData( (card_n >> 24) &0xff);
	USART_to_SendData( (card_n >> 16) &0xff);
	USART_to_SendData( (card_n >> 8) &0xff);
	USART_to_SendData(card_n & 0xff);
	USART_to_SendData(a >> 8);
	USART_to_SendData(a & 0xff);
	while (!(USART1->ISR & USART_ISR_TC));
	WR_off();

}
int main(void)
{
	init();
	flag = 0;
	lock = 0;	
	
	*stat = 0; // замок в окрытом состоянии
	if (Read16ParamFlash(0) == 0xffff){
		USART_to_SendData(0xaa);
		Write32ParamFlash(0x7801, 0);
	}
	
	if (Read16ParamFlash(2) == 0xffff){
		*t_up_m = 350;	
	}
	else {
		*t_up_m = Read16ParamFlash(2);
	}
	if (Read16ParamFlash(3) == 0xffff){
		*t_down_m = 300;
	}
	else{
		*t_down_m = Read16ParamFlash(3);
	}
	
	*adr_pcb = Read16ParamFlash(0);
	
	WR_on();
	
	USART_to_SendData(*adr_pcb >> 8);
	USART_to_SendData(*adr_pcb & 0xff);
	
	USART_to_SendData(*t_up_m >> 8);
	USART_to_SendData(*t_up_m & 0xff);
	
	USART_to_SendData(*t_down_m >> 8);
	USART_to_SendData(*t_down_m & 0xff);
	
	while (!(USART1->ISR & USART_ISR_TC));
	WR_off();
	
		
	lock = 1;		// замок в закрытом состоянии
	all_off();
	
	while(1) {
			
		// если установлена перемычка
		if (!(GPIOA->IDR & (key_open))){
			open_lock ();
			set_t(2);
		}
		else if (!(GPIOA->IDR & (key_close))){
			close_lock ();
			set_t(2);
		}

		if (flag_1) {
			if (flag_1 | 1) {		// если мотор закончил работу
				flag_1 &=~1;
				
				WR_on();
				USART_to_SendData(send_buf[0]);
				USART_to_SendData(send_buf[1]);
				USART_to_SendData(send_buf[2]);
				USART_to_SendData(send_buf[3]);
				USART_to_SendData(send_buf[4]);
				USART_to_SendData(send_buf[5]);
				USART_to_SendData(send_buf[6]);
				USART_to_SendData(send_buf[7]);
				USART_to_SendData(*adr_pcb >> 8);
				USART_to_SendData(*adr_pcb & 0xff);
				while (!(USART1->ISR & USART_ISR_TC));
				WR_off();
			}		
		}
	}
}

void EXTI0_1_IRQHandler(void)
{
	EXTI->PR |= EXTI_PR_PR1;	
	EXTI->PR |= EXTI_PR_PR0;		
}

void EXTI2_3_IRQHandler(void)
{	
	EXTI->PR |= EXTI_PR_PR3 | EXTI_PR_PR2;	
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %drn", file, line) */
 
  /* Infinite loop */
  while (1);
}
#endif
