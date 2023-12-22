#include "usart.h"
#include "main.h"
#include "main1.h"
#define BAUDRATE 57600UL

char Data = 0;
 
void Init_usart(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  //Включаем тактирование USART1

	GPIOA->AFR[1]= (1<<(2*4))|(1<<(1*4)); // или что то же самое GPIOA->AFR[1]|= 0x0110; записываем 0b0001 
	// в AFRH10 и AFRH9 (UART1 для пинов PA9 и PA10 соответствует AF1, AF1 соответствует 0b0001)

	RCC->CFGR3 &= ~RCC_CFGR3_USART1SW;
	RCC->CFGR3 |=  RCC_CFGR3_USART1SW_0;  //System clock (SYSCLK) selected as USART1 clock
	
	//Включаем USART1, Включаем приемник и передатчик USART1, Разрешаем прерывание по приему и по таймауту
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_RTOIE;
	USART1->CR1 &= ~USART_CR1_M; //Данные - 8 бит
	USART1->CR2 &= ~USART_CR2_STOP; //1 стоп-бит
	USART1->CR2 |= USART_CR2_RTOEN;			// разрешение таймаута
	USART1->RTOR |= 35;
	USART1->BRR =(APBCLK+BAUDRATE/2)/BAUDRATE; //скорость usart
	
	NVIC_SetPriority(USART1_IRQn, 15); // Как я понял, это функция, которая позволяет изменить приоритет прерывания 
	//(вот только, что происходит с тем прерыванием, которое имело этот приоритет по умолчанию я не знаю)
	NVIC_EnableIRQ(USART1_IRQn); // А эта функция, как я понял, разрешает прерывание от USART1	
}

void USART_SendData(uint16_t Data1)
{
	while(!(USART1->ISR & USART_ISR_TC));
	USART1->TDR = (Data1 & (uint16_t)0x01FF);
}

void USART1_IRQHandler(void) // Обработчик прерывания по приему символа USART
{
	if((USART1->ISR & USART_ISR_RXNE)!=0) {		
		Data = USART1->RDR; // Сбрасываем флаг (RDR не пуст, принят символ)
		//USART_SendData(Data);
	}
	
	
	else if (( USART1->ISR & USART_ISR_RTOF) !=0 ) {		// прерывание по таймауту
		USART1->ICR |= USART_ICR_RTOCF;	// сброс флага таймаута
	}
}
