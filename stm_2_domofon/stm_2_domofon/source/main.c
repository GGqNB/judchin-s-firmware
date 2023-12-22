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

uint32_t card, card2;			// id карты доступа
__IO uint32_t timeout_card1;	// таймаут карты доступа (2 бита)
__IO uint32_t timeout_card2;	// таймаут карты доступа (2 бита)

//uint16_t c_door;

uint8_t cnt_card, n_card, n_card2, cnt_card2, cnt2_card;				// счетчик бит карты доступа
uint8_t parity_card, parity_card2;			// контроль четности карты доступа (2 бита)

uint8_t flag_card, flag_card2;				// флаги состояний карты доступа
uint8_t parity_card_low, parity_card_high, parity_card_low2, parity_card_high2;

uint8_t lock;					// состояние замка

uint8_t flag_1 = 0;

extern uint8_t send_buf[8];
extern uint16_t t;

uint16_t adr;
uint8_t t_val2;

/*******************************************************************/
void init_gpio (void)
{
	// Включить тактирование порта A
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOFEN;
    GPIOA->MODER = 	GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0 |
					GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
	GPIOA->PUPDR = GPIO_PUPDR_PUPDR14_0;
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
    //EXTI->RTSR 	|=EXTI_RTSR_TR0;	// Прерывание по нарастанию уровня на ноге 0 порта привязанного к EXTI
	EXTI->FTSR |=EXTI_FTSR_TR1 | EXTI_RTSR_TR0;		// Прерывание по спаду уровня на ноге 0 и 1 порта привязанного к EXTI
    NVIC_SetPriority(EXTI0_1_IRQn, 0); // функция, которая позволяет изменить приоритет прерывания 
	NVIC_EnableIRQ(EXTI0_1_IRQn);   
	
	// ********* PA.5, PA.6 ************
	SYSCFG->EXTICR[1]|= SYSCFG_EXTICR2_EXTI6 | SYSCFG_EXTICR2_EXTI5; //     http://eugenemcu.ru/publ/13-1-0-80
    // Разрешаем прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR6 | EXTI_IMR_MR5;  
    EXTI->FTSR 	|=EXTI_FTSR_TR6 | EXTI_FTSR_TR5;	// Прерывание по спаду уровня на ноге 2, 3 порта привязанного к EXTI
    NVIC_SetPriority(EXTI4_15_IRQn, 0); // функция, которая позволяет изменить приоритет прерывания 
	NVIC_EnableIRQ(EXTI4_15_IRQn);   
}

void init (void)
{
    init_gpio();
	Init_usart();
	init_EXTI();
	SysTick_conf(SystemCoreClock/100000 - 1); // прерывание каждые 10 мкс
	init_TIM3();
	init_TIM14();
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
	USART_SendData(par >> 8);
	USART_SendData(par & 0xff);
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData( (card_n >> 24) &0xff);
	USART_SendData( (card_n >> 16) &0xff);
	USART_SendData( (card_n >> 8) &0xff);
	USART_SendData(card_n & 0xff);
	USART_SendData(a >> 8);
	USART_SendData(a & 0xff);
	while (!(USART1->ISR & USART_ISR_TC));
	WR_off();

}
int main(void)
{
	uint8_t i, test, w_card;
	uint16_t x, y;
	uint32_t z;
	uint32_t card_list[100];
	uint32_t card_temp;
	
	// uint32_t card_list[] = {0xA3B0CF, 0x900156, 0x731B16, 0x01AFD89B}; // ДК Октябрь
	//uint32_t card_list[] = {0x001, 0xe2cd9a, 0xAFD89B, 0xB2236E, 0xEA8C08, 0x7123F7, 0xA3B0CF, 0xCC8D74, 0xD60288}; // тестовые карты
	// 0xB2236E

	init();
	if (Read16ParamFlash(0) == 0xffff){
		Write32ParamFlash(0x09097901, 0);		// адрес 0x7901
	}
	adr = Read16ParamFlash(0);
	WR_on();
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData(0);
	USART_SendData(t_val1);
	USART_SendData(t_val);
	
	USART_SendData(adr >> 8);
	USART_SendData(adr & 0xff);
	while (!(USART1->ISR & USART_ISR_TC));
	WR_off();
	
	
	timeout_card1 = timeout_card_val1;	// таймаут карты доступа (2 бита)
	timeout_card2 = timeout_card_val2;	// таймаут карты доступа (2 бита)
	
	
	//считываем список карт из Flash
	for (i=1; i<100; i++){
		x = Read16ParamFlash( i << 1);
        y = Read16ParamFlash( (i << 1) + 1);
		z = (y<<16) | x;			
		if (z == 0xffffffff) {
			n_card = i-1;
			USART_SendData(0x55);
			USART_SendData(n_card);
			break;
		}
		else {
			if (z != 0){
				card_list [i-1] = z & 0xffffffff;
			}
			USART_SendData(8);
			USART_SendData(i-1);
			USART_SendData(0);
			USART_SendData(0);
			USART_SendData( (card_list [i-1] >> 24) &0xff);
			USART_SendData( (card_list [i-1] >> 16) &0xff);
			USART_SendData( (card_list [i-1] >> 8) &0xff);
			USART_SendData(card_list [i-1] & 0xff);
			USART_SendData(adr >> 8);
			USART_SendData(adr & 0xff);
			while (!(USART1->ISR & USART_ISR_TC));
		}
	}
	
	lock = 1;		// замок в закрытом состоянии
	
	while(1) {
		test = 0;
			
		if (flag_card && (card != 0)){
			
			flag_card = 0;
			//card &= 0xffffff;
			if (cnt2_card < 30) {
				card = card >> 1;
			}
			cnt2_card = 0;
			
			//card_temp = card << 24 | (card & 0xff00) << 8 | ((card & 0xff0000) >> 8) | (card >> 24);
			card_temp = card;
			
			// если установлена перемычка
			if (!(GPIOA->IDR & (1<<14))){
				//USART_SendData(0x55);
				w_card = 0;
				for (i=0; i < n_card ; i++){
					if (card_temp == card_list[i]){
						w_card = 1;
						test = 0xff;
					}
				}
				if (!w_card){
					if (Write32ParamFlash(card_temp, n_card+1)) {
						
						card_list[n_card] = card_temp;
						if (n_card < 100){
							n_card ++;
						}
						test = n_card;
					}
				}
			}
			else {
				test = 0;
				
				for (i=0; i< (n_card  ); i++){
					if ((card_temp & 0xffffff) == (card_list[i] & 0xffffff)){
						test = 1;
						clr_flag_1(1);		// ответ будет готов после окончания работы мотора
						if (lock){			// замок в закрытом состоянии 
							open_lock ();
						}
						else {
							close_lock ();
						}
						//Motor_On();
						set_t(Read16ParamFlash(1) & 255);
						
						send_buf[1] = lock;
						break;
					}
				}
			}
			WR_on();
			USART_SendData(7);
			USART_SendData(test);
			USART_SendData(0);
			USART_SendData(0);
			USART_SendData(card & 0xff);
			USART_SendData( (card >> 8) &0xff);
			USART_SendData( (card >> 16) &0xff);
			USART_SendData( (card >> 24) &0xff);
			USART_SendData(adr >> 8);
			USART_SendData(adr & 0xff);
			while (!(USART1->ISR & USART_ISR_TC));
			WR_off();

			card = 0;
			card_temp  = 0;
			cnt_card = 0;
		}
		if (flag_1) {
			if (flag_1 & 1) {		// если мотор закончил работу
				flag_1 &=~3;		// заодно очищаем возможную команду от кнопки
				adr = Read16ParamFlash(0);
				WR_on();
				USART_SendData(send_buf[0]);
				USART_SendData(send_buf[1]);
				USART_SendData(send_buf[2]);
				USART_SendData(send_buf[3]);
				USART_SendData(send_buf[4]);
				USART_SendData(send_buf[5]);
				USART_SendData(send_buf[6]);
				USART_SendData(send_buf[7]);
				USART_SendData(adr >> 8);
				USART_SendData(adr & 0xff);
				while (!(USART1->ISR & USART_ISR_TC));
				WR_off();
				send_buf[1] = 0;
				send_buf[2] = 0;
				send_buf[3] = 0;
				send_buf[4] = 0;
				send_buf[5] = 0;
			}
			if (flag_1 & 2){		// нажата кнопка
				if (lock){			// замок в закрытом состоянии 
					open_lock ();
				}
				else {
					close_lock ();
				}
				set_t(Read16ParamFlash(1) & 255);
				flag_1 &= ~2;
				//send_buf[1] = lock;
			}
		}
		
	}
}


void EXTI0_1_IRQHandler(void)
{
	//_Delay_us(100);
	if (!(GPIOA->IDR & 2)){		// если пришло "1"
		if (cnt_card2 == 0) {
			card2 = 0;
			flag_card2 = 0;
			parity_card2 = 1;
		}
		else if (cnt_card2 < 33){
			card2 = (card2 << 1) | 1;
		}
		else {
			parity_card2 |= 2;
		}
	}
	if (!(GPIOA->IDR & 1)){
		if (cnt_card2 == 0) {
			card2 = 0;
			parity_card2 = 0;
		}
		else if (cnt_card2 < 33){
			card2 = card2 << 1;
		}
		
	}
	
	cnt_card2 ++;
	timeout_card2 = timeout_card_val2;
	EXTI->PR |= EXTI_PR_PR0 | EXTI_PR_PR1;
	//EXTI->PR |= EXTI_PR_PR14;	
}

// Обработка событий от считывателя MIFARE
void EXTI2_3_IRQHandler(void)
{
	//_Delay_us(100);
	if (!(GPIOA->IDR & 8)){		// если пришло "1"
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
		cnt_card ++;
		timeout_card1 = timeout_card_val1;
	}
	if (!(GPIOA->IDR & 4)){
		if (cnt_card == 0) {
			card = 0;
			flag_card = 0;
			parity_card = 0;
		}
		else if (cnt_card < 33){
			card = card << 1;
		}
		cnt_card ++;
		timeout_card1 = timeout_card_val1;
		
	}
	
	EXTI->PR |= EXTI_PR_PR3 | EXTI_PR_PR2;	
}



void set_error_card (void)
{
}

void set_timeout_card(void)
{
	if (cnt_card > 23){
		flag_card = 1;
		cnt2_card = cnt_card;
		cnt_card = 0;
	}
	cnt_card = 0;
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
