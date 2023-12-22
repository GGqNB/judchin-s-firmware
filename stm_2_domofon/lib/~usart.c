#include "usart.h"
#include "main.h"
#include "main1.h"
#include "param_flash.h"
#include "soft_uart.h"

//#define APBCLK   48000000UL
#define BAUDRATE 9600UL

char Data = 0;
unsigned char cnt;
extern unsigned char flag_1;
extern unsigned char buf[20];
extern uint32_t SystemCoreClock;
extern unsigned char ram_parametrs[128];

void Init_usart(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  //Включаем тактирование USART1

	GPIOA->AFR[1]= (1<<(2*4))|(1<<(1*4)); // или что то же самое GPIOA->AFR[1]|= 0x0110; записываем 0b0001
	// в AFRH10 и AFRH9 (UART1 для пинов PA9 и PA10 соответствует AF1, AF1 соответствует 0b0001)

	RCC->CFGR3 &= ~RCC_CFGR3_USART1SW;
	RCC->CFGR3 |=  RCC_CFGR3_USART1SW_0;  //System clock (SYSCLK) selected as USART1 clock

	//Включаем USART1, Включаем приемник и передатчик USART1, Разрешаем прерывание по приему, по передачи и по таймауту
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_RTOIE | USART_CR1_TCIE;
	USART1->CR1 &= ~USART_CR1_M; //Данные - 8 бит
	USART1->CR2 &= ~USART_CR2_STOP; //1 стоп-бит
	USART1->CR2 |= USART_CR2_RTOEN;			// разрешение таймаута
	USART1->RTOR |= 35;
	USART1->BRR =(SystemCoreClock+BAUDRATE/2)/BAUDRATE; //скорость usart

	NVIC_SetPriority(USART1_IRQn, 15); // Как я понял, это функция, которая позволяет изменить приоритет прерывания
	//(вот только, что происходит с тем прерыванием, которое имело этот приоритет по умолчанию я не знаю)
	NVIC_EnableIRQ(USART1_IRQn); // А эта функция, как я понял, разрешает прерывание от USART1
}

void USART_SendData(uint16_t Data1)
{
	while(!(USART1->ISR & USART_ISR_TXE));
	GPIOF->BSRR = GPIO_BSRR_BS_0;
	USART1->TDR = (Data1 & (uint16_t)0x01FF);
}

void USART1_IRQHandler(void) // Обработчик прерывания по приему символа USART
{
	if((USART1->ISR & USART_ISR_RXNE)!=0) {
		Data = USART1->RDR; // Сбрасываем флаг (RDR не пуст, принят символ)
		send_soft_uart (Data);
		/*
		if (ReadParamFlash(debug)){
			USART_SendData(Data);
		}	
		//USART_SendData(Data);
		
		buf[cnt] = Data;
		cnt++;
		if ( cnt == 5 ){ // если буффер заполнен
			cnt = 0;
			if (buf[0] == ReadParamFlash(adr) ){     // если данные нам.
				// ******************************************
								// ***** общие команды для всех *****
				if (buf[2] == 1){					// запрос версии
					buf[3] = 0;
					buf[4] = version;
				}
				else if (buf[2] == 0x0F){	// пинг
				}
				else if (buf[2] == 0x16) {
					if (buf[3] < 128) {
						WriteParamFlash(buf[3],buf[4]);
						buf[4] = ReadParamFlash(buf[3]);
					}
					else {
						ram_parametrs[buf[3] - 128] = buf[4];
						buf[4] = ram_parametrs[buf[3] - 128] = buf[4];
					}
				}
				else if (buf[2] == 0x17){ 	// считать параметр из FLASH-памяти
					if (buf[3] < 128) {
						buf[4] = ReadParamFlash(buf[3]);
					}
					else{
						buf[4] = ram_parametrs[buf[3]-128];
						ram_parametrs[buf[3]-128] = 0;
					}
				}
				else {
					// ***** остальные команды уникальные для устройств *****
					commands();
				}
				buf[0] = buf[1];        		//  адрес mastera
				buf[1] = ReadParamFlash(adr);	// задаем отправителя (себя)
				buf[2] |= 128;					// для ответа устанавливаем старший бит в поле команда.

				//**************************
				USART_SendData(buf[0]);
				USART_SendData(buf[1]);
				USART_SendData(buf[2]);
				USART_SendData(buf[3]);
				USART_SendData(buf[4]);
			}
			else {
				flag_1 = 2; 		// отправка команды прочему устройству через механизм ретранслятора
			}
		}
		else if (cnt > 5) {		// на всякий случай, потом заменить на обработчик таймаута
			cnt = 0;
		}
	*/
	}

	if (( USART1->ISR & USART_ISR_RTOF) !=0 ) {		// прерывание по таймауту
		USART1->ICR |= USART_ICR_RTOCF;	// сброс флага таймаута
		cnt = 0;
	}
	
	if (( USART1->ISR & USART_ISR_TC) !=0 ) {		// прерывание по окончанию передачи
		USART1->ICR |= USART_ICR_TCCF;	// сброс флага прерывания по окончанию передачи
		GPIOF->BSRR = GPIO_BSRR_BR_0;
	}
}
