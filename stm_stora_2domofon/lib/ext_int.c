#include "main.h"
#include "usart.h"
#include "main1.h"
#include <delay.h>
#include "ext_int.h"


int check_new_buf (void)	// проверить буфер на повтор
{
	return 1;
}

void send_p (unsigned char* buf_s)
{
}

void EXTI4_15_IRQHandler(void)
{
	//_Delay_us(100);
	if (!(GPIOA->IDR & 32)){		// если пришло "1"

	}
	if (!(GPIOA->IDR & 64)){
	}

	EXTI->PR |= EXTI_PR_PR6 | EXTI_PR_PR5;
	//EXTI->PR |= EXTI_PR_PR14;	
}
