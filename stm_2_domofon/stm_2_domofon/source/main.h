#include <stm32f0xx.h>

#define SPI1_DR_8b (*(__IO uint8_t *)((uint32_t)SPI1 + 0x0C))

// Погасить светодиод
#define led_Off()		GPIOA->BSRR = GPIO_BSRR_BS_3;  // Установить бит 3
// Засветить светодиод
#define led_On()		GPIOA->BSRR = GPIO_BSRR_BR_3;  // Сбросить бит 3

// выключить нагрузку
// Направление вращения двигателя "вперед"
#define Direction_up()		GPIOA->BSRR = GPIO_BSRR_BR_3;  // Сбросить бит 3
// Направление вращения двигателя "назад"
#define Direction_down()	GPIOA->BSRR = GPIO_BSRR_BS_3;  // Установить бит 3

#define all_off()	GPIOA->BSRR = GPIO_BSRR_BR_3 | GPIO_BSRR_BR_2;  // Сбросить бит 6, 7

// управление реле
#define rele_On()		GPIOA->BSRR = GPIO_BSRR_BS_2;  // Установить бит 7
#define rele_Off()		GPIOA->BSRR = GPIO_BSRR_BR_2;  // Установить бит 7

#define WR_on()		GPIOF->BSRR = GPIO_BSRR_BS_0;  // Установить бит 1
// Засветить светодиод
#define WR_off()	GPIOF->BSRR = GPIO_BSRR_BR_0;  // Сбросить бит 1

#define t_val Read16ParamFlash(1) & 255
#define t_val1 Read16ParamFlash(1) >> 8

#define timeout_card_val1 50000
#define timeout_card_val2 50000

#define APBCLK   48000000UL
#define CE_H()   GPIOA->BSRR = GPIO_BSRR_BS_13;
#define CE_L()   GPIOA->BSRR = GPIO_BSRR_BR_13;

#define key0_1	0x40C45701
#define key0_2  0xE0000001

void init_gpio(void);
void init(void);
int main(void);
void set_error_card (void);
void set_timeout_card(void);
void set_flag_1 (int16_t f);
void clr_flag_1 (int16_t f);


