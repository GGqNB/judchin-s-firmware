#include "usart.h"
#include "main.h"
#include "main1.h"
#include "param_flash.h"
#include "modbus.h"
//#define APBCLK   48000000UL
#define BAUDRATE 9600UL

uint8_t cnt;
uint8_t cnt_send; 

uint8_t buf[10];

//extern unsigned char flag_1;

extern uint32_t SystemCoreClock;
extern unsigned char ram_parametrs[128];

extern UART_DATA uart1;//структуры для усарта

// переменные для буфера fifo
#define size_fifo 100
char fifo[size_fifo];
char ff_0;	// индекс писателя
char ff_1;	// индекс читателя
char len_ff;	// 
// ***************************

char ready_pr = 0;
char* bb;

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
	USART1->BRR =(SystemCoreClock+BAUDRATE/2)/BAUDRATE; //скорость usart

	NVIC_SetPriority(USART1_IRQn, 15); // Как я понял, это функция, которая позволяет изменить приоритет прерывания
	//(вот только, что происходит с тем прерыванием, которое имело этот приоритет по умолчанию я не знаю)
	NVIC_EnableIRQ(USART1_IRQn); // А эта функция, как я понял, разрешает прерывание от USART1
}

void USART_SendData(uint16_t Data1)
{
	//while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (Data1 & (uint16_t)0x01FF);
}
int print_fifo (char* buffer, char len)
{
	char i;
	if ( (size_fifo - ff_0) > len ) {
		for (i=0; i<len; i++) {
			fifo[ff_0] = buffer[i];
			ff_0++;
		}
	}
	else {
		for (i=0; i<(size_fifo - ff_0); i++) {
			fifo[ff_0] = buffer[i];
			ff_0++;
		}
		ff_0 = 0;
		while (i < len){
			fifo[ff_0] = buffer[i];
			ff_0++;
			i++;
		}
	}
	return 1;
}




// функция отправляет buf[] через уарт
void print_buf (char* buffer)
{
	while (ready_pr);
	bb = buffer;
	cnt_send = 0;
	USART_SendData(buffer[cnt_send]);
	//Разрешаем прерывание по опустошению буфера
	USART1->CR1 |= USART_CR1_TXEIE;
	ready_pr = 1;
}

void USART1_IRQHandler(void) // Обработчик прерывания по приему символа USART
{
	uint8_t Data;
	
	if((USART1->ISR & USART_ISR_RXNE)!=0) {
		Data = USART1->RDR; // Сбрасываем флаг (RDR не пуст, принят символ)
		buf[cnt] = Data;
		cnt++;		
		
		uart1.buffer[uart1.rxcnt] = Data; // Сбрасываем флаг, чтением принятого символа (RDR не пуст, принят символ) 
		uart1.rxcnt++;	
		uart1.rxcnt = uart1.rxcnt % 256; // защита от переполнения буфера
		
	}	
		
	else if (( USART1->ISR & USART_ISR_RTOF) !=0 ) {		// прерывание по таймауту
		USART1->ICR |= USART_ICR_RTOCF;
		if (cnt == 10){
			commands(buf);		
			cnt = 0;
			buf[8] = 0;
			buf[9] = 0;
		}
		else {
			modbus_slave();
		}		
	}
	
	// ***** прерывание по окончанию передачи *****
	else if (( USART1->ISR & USART_ISR_TC) !=0 ) {		
		uart1.txcnt ++;
		if (uart1.txlen == uart1.txcnt) {
			USART1->CR1 &= ~USART_CR1_TCIE; // запрещаем прерывание по отправке 
			USART1->CR1 |= USART_CR1_RE;	// разрешаем прерывание по приему
			
			USART1->ICR |= USART_ICR_TCCF;	// сброс флага если больше нечего передавать
			WR_off();
		}
		else {
			USART_SendData (uart1.buffer[uart1.txcnt]);
		}
	}
}
