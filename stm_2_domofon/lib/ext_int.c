#include "main.h"
#include "usart.h"
#include "main1.h"
#include <delay.h>
#include "ext_int.h"


extern uint32_t card;			// id карты доступа
extern __IO uint32_t timeout_card1;	// таймаут карты доступа (2 бита)
extern __IO uint32_t timeout_card2;	// таймаут карты доступа (2 бита)

//uint16_t c_door;

extern uint8_t cnt_card;				// счетчик бит карты доступа
extern uint8_t parity_card;			// контроль четности карты доступа (2 бита)
extern uint8_t flag_card;				// флаги состояний карты доступа
extern uint8_t parity_card_low, parity_card_high;



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
	if (!(GPIOA->IDR & 64)){		// если пришло "1"
		if (cnt_card == 0) {
			card = 0;
			flag_card = 0;
			parity_card = 1;
		}
		else if (cnt_card < 33){
			card = (card << 1) | 1;
		}
		else {
			parity_card |= 2;
		}
	}
	if (!(GPIOA->IDR & 32)){
		if (cnt_card == 0) {
			card = 0;
			parity_card = 0;
		}
		else if (cnt_card < 33){
			card = card << 1;
		}
		
	}
	
	cnt_card ++;
	timeout_card1 = timeout_card_val1;
	EXTI->PR |= EXTI_PR_PR6 | EXTI_PR_PR5;
	//EXTI->PR |= EXTI_PR_PR14;	
}
