#include <stm32f0xx.h>

void Init_usart(void);
void USART_SendData(uint16_t Data1);
void USART1_IRQHandler(void); // Обработчик прерывания по приему символа USART
